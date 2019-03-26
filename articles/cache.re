= キャッシュでドハマリ

本章のTL;DR

 * @<code>{cache.writeData}は極力使うな
 * IDを定めにくいデータをキャッシュに書くには@<code>{cache.writeQuery}を使え
 * IDが決められるデータをキャッシュに書くには@<code>{cache.writeFragment}を使え
 * Apolloでドハマリしそうになったらコード読んでデバッガ使って処理を追いかけろ

== キャッシュ & ローカル状態管理

キャッシュというかローカルの状態管理をApolloでしようとした話なんですが、これらは切り離せない話題であります。

apollo-client 2.5以前ではapollo-link-state@<fn>{apollo-link-state}というパッケージとして独立していたようですが2.5からは本体に取り込まれたらしいです。
最近参入勢なのでその辺はあまり知らない…！

本章で@<code>{cache}という用語が出てきた時は、@<code>{apoll-cache}@<fn>{npm-apollo-cache}ないし@<code>{apollo-cache-inmemory}@<fn>{npm-apollo-cache-inmemory}のインスタンスのことを指します。
もっぱら@<code>{new InMemoryCache()}で得られるものですね。

//footnote[npm-apollo-cache][@<href>{https://www.npmjs.com/package/apollo-cache}]
//footnote[npm-apollo-cache-inmemory][@<href>{https://www.npmjs.com/package/apollo-cache-inmemory}]

ローカルの状態管理をApolloでやること自体については公式のドキュメント@<fn>{apollo-local-state-1}@<fn>{apollo-local-state-2}を参照してください。
これらドキュメント中で@<code>{cache.writeData}を使う例が出てきますが、後述する理由により罠だと考えていますのでそこだけ注意してください。

//footnote[apollo-link-state][@<href>{https://www.npmjs.com/package/apollo-link-state}]
//footnote[apollo-local-state-1][@<href>{https://www.apollographql.com/docs/tutorial/local-state.html}]
//footnote[apollo-local-state-2][@<href>{https://www.apollographql.com/docs/react/essentials/local-state.html}]

基本的にローカルの状態管理は、ネットワーク経由でAPIを叩いた結果と同一のキャッシュに載ります。
つまり、キャッシュが一種のDBとして機能するわけです。
QueryやらMutationやらでローカルの状態変更を行うのは、それらの操作を通してキャッシュをread、writeしているに過ぎません。

というわけで、キャッシュについての公式ドキュメント@<fn>{apollo-caching}を読みましょう。
このドキュメントに書いてあることは信じてよいと思います。
@<code>{cache.writeData}出てこないし…！

//footnote[apollo-caching][@<href>{https://www.apollographql.com/docs/react/advanced/caching.html}]

== cache.writeData, Query, Fragment

ハマりの説明に入りましょう。
筆者が作っていた（いる）アプリは、任意のデータベースの中身を引っ張ってきてJSONとして取得できる動作が含まれています。
任意のデータベースなので、もちろんスキーマは一定ではありません。
そのため、@<code>{scalar JSON}というcustom scalar typeを作成しました（@<list>{code/cache-example/schema.graphql}）。

//list[code/cache-example/schema.graphql][こんなスキーマだと思って]{
#@mapfile(../code/cache-example/schema.graphql)
type Query {
  cat(id: ID!): Cat
  cats: [Cat!]
}

scalar JSON

type Cat {
  id: ID!
  kind: String
  name: String!
  data: JSON
}
#@end
//}

公式のドキュメント@<fn>{apollo-custom-scalar-type}にもこのやり方が（サーバ側に関してのみ）紹介されています。
うーん、なるほど。

//footnote[apollo-custom-scalar-type][@<href>{https://www.apollographql.com/docs/graphql-tools/scalars.html}]

更に調べていくとクライアント側のcustom scalar typeサポートはまだ存在していません@<fn>{apollo-client-side-custrom-scalar}。
サポートされていない、というのは値の形式変換が暗黙理にできないというだけで、実用上少し苦労する程度で済みます。

//footnote[apollo-client-side-custrom-scalar][@<href>{https://github.com/apollographql/apollo-feature-requests/issues/2}]

問題になるのは、キャッシュに対してこの形式のデータを書こうとした時です。
ありがちな理由として、画面の左側にリストがあって、そこからアイテムを選択し、右側で編集したいとします。
この操作を画面上で実現するためには、選んだアイテムの情報をどこかに保持しておかなければなりません。
画面初期化時に、適当なデータをキャッシュに書き込む場合もあるでしょう。

書き込み処理として、@<code>{cache.writeData}はスキーマレスなデータ@<strong>{しか}渡すことができません。
また、cacheもclientも実行時にはスキーマに関する情報を保持していません（かなり盲点ですね）。

さて、キャッシュのデータは必ずスキーマとセットで扱われます。
@<code>{cache.writeData}では、このギャップを埋めるために@<code>{queryFromPojo}というユーティリティ関数を使って、データからスキーマをひねり出して使います。
しかしながら、渡したデータのどの部分が@<code>{scalar JSON}なのかがわからないため、JSONの中まで見に行って@<code>{__typename}とか@<code>{id}がなくて大騒ぎになります（@<list>{code/cache-example/src/writeData.ts}）。

//list[code/cache-example/src/writeData.ts][writeDataは危険なのです]{
#@mapfile(../code/cache-example/src/writeData.ts)
import { InMemoryCache } from "apollo-cache-inmemory";

const cache = new InMemoryCache({
  // Relay Global Object Identification に従い、IDだけで一意になるようにする
  // InMemoryCacheのデフォルトだと `${obj.__typename}:${obj.id}` などが使われる
  dataIdFromObject: obj => obj.id,
});

// 次のような __typename がないぞ！というエラーが2回出る
// Missing field __typename in {
//   "date": "2019/03/26",
//   "event": "爪が伸びたのでかいぬしをプスってした"
// }
cache.writeData({
  data: {
    notExists: true, // schemaに存在しないデータもお咎めなし
    cats: [
      {
        __typename: "Cat",
        id: "Cat:yukari",
        kind: "norway jan forest cat & ragdoll",
        name: "yukari",
        data: [
          {
            date: "2019/03/26",
            event: "爪が伸びたのでかいぬしをプスってした",
          },
          {
            date: "2019/03/27",
            event: "あくびしてるところの写真を撮られた",
          },
        ],
      },
    ],
  },
});

// 内部的に作成されたキャッシュは次の通り…
// ROOT_QUERY
// Cat:yukari
// Cat:yukari.data.0 ←余計 JSONの中まで見てる
// Cat:yukari.data.1 ←余計 JSONの中まで見てる
Object.keys(cache.extract()).forEach(cacheKey => console.log(cacheKey));
#@end
//}

この挙動はまったくもってtype safeではありません。
ミスがあっても検出できないし、キャッシュの中身はぐちゃぐちゃになるし、最悪です。
人類がこんなことで苦しまなければいけないのは馬鹿げていますね。

この挙動はドキュメントには明記されていないように見えますし、ひたすらデバッガ片手に処理を追いかけていかないと原因がわかりませんでした。
最終的に、設計思想と自分の実装にミスマッチがありうまく行っていないことがわかるまで非常に苦労しました。
壊れたデータを書いてしまった場合でも警告などが得られないパターンもあり、それなのに一見関係ないQueryで結果がおかしくなったりして本当に罠みが高いです。

@<code>{cache.writeData}は罠！GraphQLでコードを書くというのにスキーマレスになりうるパーツを使うのは罪！
そのことを強く胸に刻みこんで生きていきましょう。
Issueも書いてみた@<fn>{apollo-issue-4554}のですが、反応まるでないし@<code>{cache.writeQuery}か@<code>{cache.writeFragment}使え、で終わりそうではあります。

//footnote[apollo-issue-4554][@<href>{https://github.com/apollographql/apollo-client/issues/4554} まぁ筋の良い話ではなさそう]

どういう方向性で実装するのがよかったかを解説します。
@<code>{cache.writeFragment}か@<code>{cache.writeQuery}を常に使うのが正しいです。
FragmentかQueryを与えると、書き込みたいデータに対してスキーマとして機能し、それに沿ってデータ形式が検査されます。
つまり、FragmentやQueryをバリデータとして与えるという意味合いになります。
これらは@<code>{apollo client:codegen}を使うことでデータがスキーマに対して正しい形式であることを間接的に、そして静的にチェックできます。

2つの使い分け方として、IDが判明している単一のデータの書き込みには@<code>{cache.writeFragment}を使い、そうでなければ@<code>{cache.writeQuery}を使うことになります。
とりあえず@<code>{cache.writeFragment}を使って、それが無理な場合は@<code>{cache.writeQuery}を使う、と覚えてください。
具体的に、@<list>{code/cache-example/src/writeFragment.ts}と@<list>{code/cache-example/src/writeQuery.ts}のように使います。
型パラメータを指定するのを怠るとデータの形式が正しいかどうか検証されなくなってしまうので注意しましょう。

//list[code/cache-example/src/writeFragment.ts][writeFragmentの安全な使い方]{
#@mapfile(../code/cache-example/src/writeFragment.ts)
import gql from "graphql-tag";
import { InMemoryCache } from "apollo-cache-inmemory";
import { WriteCacheFragment } from "./graphql/WriteCacheFragment";

const cache = new InMemoryCache({
  dataIdFromObject: obj => obj.id,
});

// apollo client:codegen が完走する→schemaに対して不正なQueryではないことがわかる
// apolloが生成した型を型パラメータとして利用するとdataの書式もチェックできる
cache.writeFragment<WriteCacheFragment>({
  id: "Cat:yukari",
  fragment: gql`
    fragment WriteCacheFragment on Cat {
      id
      kind
      name
      data
    }
  `,
  data: {
    __typename: "Cat",
    id: "Cat:yukari",
    kind: "norway jan forest cat & ragdoll",
    name: "yukari",
    data: [
      {
        date: "2019/03/26",
        event: "爪が伸びたのでかいぬしをプスってした",
      },
      {
        date: "2019/03/27",
        event: "あくびしてるところの写真を撮られた",
      },
    ],
  },
});

// 内部的に作成されたキャッシュは次の通り
// Cat:yukari
Object.keys(cache.extract()).forEach(cacheKey => console.log(cacheKey));
#@end
//}

//list[code/cache-example/src/writeQuery.ts][writeQueryの安全な使い方]{
#@mapfile(../code/cache-example/src/writeQuery.ts)
import gql from "graphql-tag";
import { InMemoryCache } from "apollo-cache-inmemory";
import { WriteCacheQuery } from "./graphql/WriteCacheQuery";

const cache = new InMemoryCache({
  dataIdFromObject: obj => obj.id,
});

// apollo client:codegen が完走する→schemaに対して不正なQueryではないことがわかる
// apolloが生成した型を型パラメータとして利用するとdataの書式もチェックできる
cache.writeQuery<WriteCacheQuery>({
  query: gql`
    query WriteCacheQuery {
      cats {
        id
        kind
        name
        data
      }
    }
  `,
  data: {
    // notExists: true, // ←型でエラーになる & 与えてもキャッシュに書く際に無視される
    cats: [
      {
        __typename: "Cat",
        id: "Cat:yukari",
        kind: "norway jan forest cat & ragdoll",
        name: "yukari",
        data: [
          {
            date: "2019/03/26",
            event: "爪が伸びたのでかいぬしをプスってした",
          },
          {
            date: "2019/03/27",
            event: "あくびしてるところの写真を撮られた",
          },
        ],
      },
    ],
  },
});

// 内部的に作成されたキャッシュは次の通り キレイ！
// Cat:yukari
// ROOT_QUERY
Object.keys(cache.extract()).forEach(cacheKey => console.log(cacheKey));
#@end
//}

ここでのFragmentやQueryは、clientを使って何かをリクエストする時のFragmentやQueryと同一のものではありません。
キャッシュ書き込みのための固有のパーツを用意してもいいですし、画面で使っているものを流用しても構いません。
キャッシュのデータ実体は常に1つであり、そこに対する書き込み、読み込みの型としてFragmentやQueryを流用しています。
FragmentやQuery毎にキャッシュが独立しているわけではないのです（@<list>{code/cache-example/src/queryAsASchema.ts}）。

//list[code/cache-example/src/queryAsASchema.ts][キャッシュは常に1つ]{
#@mapfile(../code/cache-example/src/queryAsASchema.ts)
import gql from "graphql-tag";
import { InMemoryCache } from "apollo-cache-inmemory";
import { WriteCacheFragment1 } from "./graphql/WriteCacheFragment1";
import { WriteCacheFragment2 } from "./graphql/WriteCacheFragment2";
import { ReadCacheFragment } from "./graphql/ReadCacheFragment";

const cache = new InMemoryCache({
  dataIdFromObject: obj => obj.id,
});

// apollo client:codegen が完走する→schemaに対して不正なQueryではないことがわかる
// apolloが生成した型を型パラメータとして利用するとdataの書式もチェックできる
cache.writeFragment<WriteCacheFragment1>({
  id: "Cat:yukari",
  fragment: gql`
    fragment WriteCacheFragment1 on Cat { id kind }
  `,
  data: {
    __typename: "Cat",
    id: "Cat:yukari",
    kind: "norway jan forest cat & ragdoll",
  },
});
cache.writeFragment<WriteCacheFragment2>({
  id: "Cat:yukari",
  fragment: gql`
    fragment WriteCacheFragment2 on Cat { kind name }
  `,
  data: {
    __typename: "Cat",
    kind: null,
    name: "yukari",
  },
});

const result = cache.readFragment<ReadCacheFragment>({
  id: "Cat:yukari",
  fragment: gql`
    fragment ReadCacheFragment on Cat {
      id
      kind
      name
    }
  `,
});
// 結果は統合される
// 書き込みフィールドが被った場合は後勝ち
// {"id":"Cat:yukari","kind":null,"name":"yukari","__typename":"Cat"}
console.log(JSON.stringify(result));
#@end
//}

== ドハマリ回避のために

皆さんにおすすめしたいのが、とりあえずハマる前にキャッシュ関係のソースコードを読め！ということです。
apollo-cacheの実装@<fn>{apollo-cache-src}を読んでみて驚くのは、そのコード量の少なさです。
めっちゃサクサク読める！
前述の@<code>{writeData}の実装や、@<code>{writeQuery}、@<code>{writeFragment}がどういう挙動をしようとしているのか把握しておくとドハマリが避けられます。

//footnote[apollo-cache-src][@<href>{https://github.com/apollographql/apollo-client/tree/61639bcf44981a879f20c6196f74a7f7244bfda4/packages/apollo-cache}]

apollo-cache-inmemoryの実装@<fn>{apollo-cache-inmemory-src}も読むとタメになりますが、優先度は低いでしょう。
ドキュメント類の網羅度が貧弱なので謎メソッドの実装を読んで振る舞いを把握する必要があります。

//footnote[apollo-cache-inmemory-src][@<href>{https://github.com/apollographql/apollo-client/tree/61639bcf44981a879f20c6196f74a7f7244bfda4/packages/apollo-cache-inmemory}]

apollo-cache-inmemoryはoptimismパッケージ@<fn>{npm-optimism}に依存しているのですが、こいつが複雑&ドキュメント皆無というド面倒なやつです。
あまりに面倒なので読むのを途中で諦めたのですが、デバッガでコードを追っている時に、apolloのコードを見ているのかoptimismのコードを見ているのか、判断できる程度に様子を見ておくと便利です。

//footnote[npm-optimism][@<href>{https://www.npmjs.com/package/optimism}]

== その他のキャッシュ関連プラクティス

試行錯誤する過程で、コレ以外にもいくつかプラクティスを発見したので紹介していきます。

=== cacheのcacheRedirectsを頼る

clientのresolversより先に、cacheのcacheRedirectsで解決できないか検討する。
発想として、クライアントローカルな処理を定義する時にclientのresolversに実装を追加して解決したくなります。
しかし、clientとcacheはレイヤーが分かれていて、アプリケーション→client→cacheと処理が流れていくことを忘れてはいけません。
cacheのレイヤーで無理なく実装できることはcacheのレイヤーで実装するのがよいでしょう。

#@# TODO この辺もうちょっと具体的な理由があった気がするけど忘れた

=== クライアント側独自の型をなるべく作らない

ローカルに状態を持つ時、可能な限り独自のtypeやらを定義しない。
単純に@<code>{__typename}やら@<code>{id}やら、そのルールを定義しメンテするのがめんどくさいからです。
そして、キャッシュの仕組み上@<code>{__typename}と@<code>{id}が定義できないような構造を導入すると痛い目を見ることになります。
フィールドがたくさんあると、初期状態に適当なデフォルト値をセットするのが面倒…というのもあるのですが、その苦労はこなしておくのが妥当です。

現状、筆者はサーバ側もクライアント側も自分で書いているのでシステム全体の一貫性を考慮し、設計するのは比較的容易です。
しかし、大規模化した時のことを考えると余計な複雑さは抑えておくべきでしょう。
筆者も最初はQuery配下に色々な要素を散らかすのはちょっと…と思い、ローカルの状態を管理する型を作ろうかと思いました。
しかし、すぐに@<code>{id}が定義できないので@<code>{cache.writeFragment}が使えず面倒になってしまいました。
代わりに、Queryの直下に@<code>{current}プリフィクスをつけたフィールドを定義し、ローカルの状態管理にあてています。

#@# TODO writeQueryで書き込むとネストした箇所の部分的な更新ができずにつらい目にあう気がするので検証しましょうね

=== @export に頼らない

@<code>{@export}を利用して頑張ろうとしない。
GraphQLはクエリであるため、関数を持ちません。
これは真面目に考えるとなかなか厄介で、@<code>{ID}を@<code>{Boolean}に変換することができません。
つまり、値の有無を見て@<code>{@skip}や@<code>{@include}の引数とすることができないのです。
変数の値の変換をどこかのレイヤーでできると良いのでしょうが、筆者の知るかぎり難しいようです。
現状、これの代わりに使っているのがcacheのcacheRedirectsを使った方法です。

#@# TODO もうちょっと実際のアプリケーションのコードやプラクティスの説明あってもよさそう currentHogeIDとcurrentHogeについてとか

//comment{
わかりにくい点。
 1. writeData でschemaに対してぶっ壊れたデータ書いても怒られない。しかし後で挙動がおかしくなって原因はわからず脳が爆発する。関係ないQueryの結果が真っ白に書き換わったりする。
 2. ローカルなresolverを定義するのは最後の手段…が良さそうかなぁ writeData でなんとかしたい
 ** 計算処理を定義するより、キャッシュ上にドンピシャなデータをwriteしてそれを保持したほうが脳が楽そう
 3. writeData ではschemaにあった形式をwriteしないといけない
 4. refetchQueriesでピタゴラスイッチしようとすると爆発四散する
 ** 話がわかりにくいけど… @client で ほげほげID を保持して別のところでほげほげIDからデータを持ってきて…としようとするとよくない
 ** ほげほげIDをセットしたら、データとして保持するのはそのIDではなくて、引っ張ってきたデータを保持する 2段階右折みたいなことしない
 ** ほげほげを編集する時ほげほげEditみたいな型をローカル側で作らない 元のデータ型のまま扱う Submit時のInputへの変形はReactのレベルで頑張る
 5. @export で頑張ったりするのはかなりつらい
 ** ID に対して null だったら Boolean! に変換するみたいなのがつらい
 ** 動的に引数が変わるようなコード書くと、query と variables の組み合わせが変わるのでrefetchQueryが意図どおり適用されない疑惑がある(勘違いの可能性もあるので要検証)
//}
