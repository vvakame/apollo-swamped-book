= Apolloのドハマリ事件簿

技術書典5ではGraphQLサーバをGo言語で作るというネタをやりました。
今回は、所変わってクライアント側を普通にReact+TypeScript+Apolloでクライアント側をやっていきつつある話をします。

今回のネタは"Apollo泣き言集"です。
ふえぇApollo難しいでしゅう…。

GraphQLの概要についてはそろそろ割愛していいと思いますので割愛します。
Google先生で調べるか、私による解説が読みたい人は@<href>{https://vvakame.booth.pm/items/1055228}を参照してください。

== TL;DR

 * @<code>{cache.writeData}は極力使うな
 * IDを定めにくいデータをキャッシュに書くには@<code>{cache.writeQuery}を使え
 * IDが決められるデータをキャッシュに書くには@<code>{cache.writeFragment}を使え
 * Apolloの魔法力を信じろ、それはきっと実現可能だ
 * Apolloの魔法を信じるな、コード読んでデバッガ使って処理を追いかけろ

== クライアントサイドにおけるGraphQLのアドバンテージ

なんといってもGraphQLは型を持っていることが最重要ポイントです。
型があるということは、静的にクエリがvalidか否か判定することができますし、クエリで得られるデータに合致する"完全な"型を得ることができます。

GitHubが提供するv4 API@<fn>{github-v4}はGraphQLで提供されていますので、これを例として見せます。
大まかな流れは次のようになります。

 1. GraphQLのスキーマのデータをどこかからひねり出す
 2. クエリを書く
 3. クエリから型定義ファイルを生成する

//footnote[github-v4][@<href>{https://developer.github.com/v4/explorer/}]

まずは1つ目のステップです。
GitHubはAPIを叩く時にAccessTokenを必要としますので、Personal Access Token@<fn>{github-pat}などを適当に準備します。

//footnote[github-pat][@<href>{https://github.com/settings/tokens}]

//cmd{
$ npx apollo schema:download --endpoint https://api.github.com/graphql --header "Authorization: Bearer ${GITHUB_TOKEN}"
  ✔ Loading Apollo Project
  ✔ Saving schema to schema.json
//}

これでGitHubのGraphQLの型情報が詰まったschema.jsonが作成されました。

次にクエリを書いてみます（@<list>{code/github-query/src/index.ts}）。

//list[code/github-query/src/index.ts][自分のIDとbioを取得するクエリ]{
#@mapfile(../code/github-query/src/index.ts)
import gql from "graphql-tag";

const viewerQuery = gql`
  query ViewerQuery {
    viewer {
      # もちろんこれ以外にも色々ある
      id
      bio
      here:location
    }
  }
`;
#@end
//}

クエリを@<code>{graphql-tag}に文字列テンプレートリテラルとして渡してやるだけです。
Fragmentの埋込なども@<code>{${otherFragmentDefinition\}}的にできます。

apolloでは、クエリやフラグメントにはしっかり名前をつけなければいけません。
この習慣はサーバ側でトレースを分析する時にも便利なので、なんらかの命名規則を設けてしっかり管理するのをおすすめします。

これをapollo先生に解析してもらい、型定義情報を出力してもらいます。

//cmd{
$ npx apollo client:codegen --localSchemaFile=./schema.json --addTypename --target=typescript --outputFlat src/graphql

  ✔ Loading Apollo Project
  ✔ Generating query files with 'typescript' target - wrote 2 files
//}

得られた型定義ファイルを見てみます（@<list>{code/github-query/src/graphql/ViewerQuery.ts}）。

//list[code/github-query/src/graphql/ViewerQuery.ts][型定義ファイルの様子]{
#@mapfile(../code/github-query/src/graphql/ViewerQuery.ts)
/* tslint:disable */
/* eslint-disable */
// This file was automatically generated and should not be edited.

// ====================================================
// GraphQL query operation: ViewerQuery
// ====================================================

export interface ViewerQuery_viewer {
  __typename: "User";
  id: string;
  /**
   * The user's public profile bio.
   */
  bio: string | null;
  /**
   * The user's public profile location.
   */
  here: string | null;
}

export interface ViewerQuery {
  /**
   * The currently authenticated user.
   */
  viewer: ViewerQuery_viewer;
}
#@end
//}

うーん、素晴らしい！
型定義にはクエリで要求したフィールドのみが出力されています。
また、nullableかどうかもキチンと処理され、必ず値があるものについてはその型が、そうではないものはnullの可能性ありと型に表れています。

このように、GraphQL+apolloではアプリケーション的に関心がある型のみを相手にできるようになっているのです。
ワンダフル！

===[column]

GraphQLの型情報を持ったファイル、と言った時、該当するものが複数あります。

1つ目が、主に拡張子.graphqlで表されるファイルで、中身は人間が読み書きするためのスキーマ定義です（@<list>{graphql-ext-example}）。

//list[graphql-ext-example][*.graphqlの例]{
type Query {
  node(id: ID!): Node
  nodes(ids: [ID!]!): [Node]!
}
//}

2つ目が、schema.jsonなどで表される型情報が集まったJSONファイルです。

型定義や型情報と言った時、この2つのうちどっちを指すのかは文脈やツールに依存します。
気合でわかってください。

主な使われ方として、.graphqlは人間が読み書きし、サーバ側実装のソースにします。
JSONファイルはサーバ側のIntrospection APIを叩き、GraphQLの実装から型情報を持ってきます。
つまり、*.graphql→サーバ→schema.jsonというフローです。

*.graphqlからサーバを経由することなしにschema.jsonにする方法もあります。
筆者も駆け出しの頃に探したけどなかなか見つからず苦労したので今使っているスクリプトのgistにあげておきます。

@<href>{https://gist.github.com/vvakame/0d92c9101e6db6fa6f5f2ab714bca00e}

===[/column]

== Apolloという魔法

GraphQLのクエリを書くと型定義が生成される！というのはかなり魔法っぽいです。
実装しろと言われたら、まぁ普通にAST見て特定のパターン見つけたらクエリと判断してパースして…と想像することはできます。
魔法の力は一体どこまで我々の想像の上をゆくのか？
強いところも弱いところもあります。
やっていきましょう。

 * 魔法すごい！
 ** コード中のQueryやらMutationを拾い上げて型定義を出力してくれる
 ** ローカルの状態管理もできる（Redux不要説）
 ** リモートとローカルへのクエリを混ぜて書ける
 *** 自動的に分解・統合を裏でやってくれる
 ** キャッシュを賢く行ってくれる
 *** データの更新がかかると関連箇所に自動反映される

素晴らしい！かもね。

 * 魔法すごくない…
 ** リストの継ぎ足し（fetchMore）が思ったより自動じゃない
 ** ちょいちょい謎のワークアラウンドがありお気持ちを忖度するのが難しい

おつらい…。

現時点の筆者の感想として、Apolloは"現実に存在する"ユースケースを広くカバーしようと頑張っています。
そのため、"あの仕様を踏襲していればこういう魔法がかかるのでは…？"という期待は打ち砕かれることが多いです。

打ち砕かれるとは言いましたが、Relay global object identification@<fn>{relay-global-obj-id}の存在はしっかり抑えておいたほうがよいです。
GraphQLにおいては、全ての非スカラなオブジェクトはグローバルなIDで識別できるのがよいです。
力強くやっていきましょう。
筆者は普通に@<code>{テーブル名:PK}ぐらいのノリでIDを作ってます。

//footnote[relay-global-obj-id][@<href>{https://facebook.github.io/relay/graphql/objectidentification.htm}]

== Apolloドハマリメモ

というわけで技術書典Webで使おうとしたり、あと社内ツールをフルスクラッチする機会を手に入れたので素振りがてら色々やってみています。
Apolloは大枠としてはわかりやすく、第一歩で躓くことは少ないように思います。

というわけで、ここからは筆者が陥ったドハマリについて解説していきたいと思います。
皆さんは私のような思いはしてほしくない…俺の屍を越えてゆけ…！
デバッグログとかもうちょっとなんか出力してくれないですかね…。

公式ドキュメントは若干長ったらしく、そのくせ網羅的な解説ではありません。
よって、使っていい仕様と使わないほうがよい仕様がわかりにくいです。
色々と理解してしまうと正しいことが書いてあることは理解できるのですが…。
掲載されているサンプルコードも抜粋が多く、全体像がわかりにくく、動作させられるサンプルがあるのかもわかりにくいです（だいたい無い印象）。

さらに、Apolloはサーバ側もカバーしているため、サーバ側の話とクライアント側の話を区別する必要があります。
特定のトピックに関して、サーバ側でのやり方は書いてあってもそれに対応するクライアント側のやり方が併記されていないことがほとんどです。
さらに、基本的にはJavaScriptの話題しか見当たりません。
ApolloはJavaやScalaやSwiftもサポートしているようなのですが、それらをやっていきたい人はさらに獣道を歩くことになるのではないでしょうか。

総じて、やりたいことをパッと実現できなかった場合、コードを読みデバッガをお供に冒険する必要があります。

=== QueryとfetchMoreの謎

まず困ったのがQueryとfetchMoreの対応関係です。
無限読み込みできるリストを作ろう！ということで作ったのですが、fetchMoreの使い方が全然わかりません。

自分のGraphQLエンドポイントはRelay Cursor Connections Specification@<fn>{relay-connections}を踏襲しているし、@<code>{@connection}って自動継ぎ足しが可能なやつ…？と思ったんですがそんなことは全然なかった。
ここは魔法を期待しすぎた箇所ですね。

//footnote[relay-connections][@<href>{https://facebook.github.io/relay/graphql/connections.htm}]

実際に必要なのは次の2ステップです。

 1. 自分で継ぎ足し分を取得するQueryを書く。
 2. 自分で取得した結果を既存の結果と合成する。

fetchMoreは自動継ぎ足しはしてくれません！
とりあえずここだけ覚えて返ってください。
fetchMoreはQueryと大きな差はなく、updateQueryで元のデータと継ぎ足し用データを合成して返すステップがあるだけです。

ということを理解するまでにめっちゃ時間がかかった…。
公式のPaginationの説明@<fn>{apollo-pagination}には必要なことが書いてあって、ここでやっていることを丸コピすればだいたいOKです。
魔法はない、ないんだ…！

//footnote[apollo-pagenation][@<href>{https://www.apollographql.com/docs/react/features/pagination.html}]

期待していたけど存在しなかった魔法を晒しておきます。

1つ目。
継ぎ足し分を取得するQueryは自動生成される。
Apolloは魔法たっぷり！だから自動でやってくれるかな？と思ったけどそんなことはなかった。
Query中のPaginationが行われている箇所はRelay Cursor Connections Specificationに従っていれば検出可能である。
複数候補がある場合でも、pathないし何らかの方法で該当箇所を明示可能である。
そうしたら、Document中の不要な箇所は自動的に削除できるはずだし、Pagingする方法は一意だし、自動で継ぎ足しクエリを生成して実行してくれるのでは？
と、いう仕様を想像すると、脳内では整合性が取れているのでそう実装されていてくれ…！@<code>{@connection}はそのための仕様か！？とか思ってしまうわけである。
思い込みってこわい。

2つ目。
得られたデータは自分で合成しないといけない。
これについてもRelay Cursor Connections Specificationに従っていれば自動的に合成可能なはずである。
でもまぁそんなことはなかった。
先入観抜きにサンプルコードを見ると普通に合成している。
思い込みってこわい。

その他、悩ましい点。
画面内にPageとComponentという区分が存在するとして、可能な限りQueryの実行はPage単位で行いたい。
よって、各ComponentはFragmentを持ち、Pageで合成することになる。
で、fetchMoreの実装コードはQueryのそばに置くか、Fragmentのそばに置くか、どっちがいいんだろ？
今のところ@<code>{<Query />}の近くに書いたほうがわかりやすいのでPage側に置いてます。

=== キャッシュとの戦い

はい。キャッシュというかローカルの状態管理の話なんですがあまり切り離せない話題ではあります。
apollo-client 2.5以前ではapollo-link-state@<fn>{apollo-link-state}というパッケージとして独立していたようですが2.5からは本体に取り込まれたらしいです。
最近参入勢なのでその辺はあまり知らない…！
ローカルの状態管理をApolloでやること自体については公式のドキュメント@<fn>{apollo-local-state-1}@<fn>{apollo-local-state-2}を参照してください。
これらドキュメント中で@<code>{cache.writeData}を使う例が出てきますが、後述する理由により罠だと考えていますのでそこだけ注意してください。

//footnote[apollo-link-state][@<href>{https://www.npmjs.com/package/apollo-link-state}]
//footnote[apollo-local-state-1][@<href>{https://www.apollographql.com/docs/tutorial/local-state.html}]
//footnote[apollo-local-state-2][@<href>{https://www.apollographql.com/docs/react/essentials/local-state.html}]

基本的にローカルの状態管理は、ネットワーク経由のAPIと同一のキャッシュに載ります。
つまり、キャッシュが一種のDBとして機能するわけです。
QueryやらMutationやらでローカルの状態変更を行うのは、それらの操作を通してキャッシュをread、writeしているに過ぎません。

というわけで、キャッシュについての公式ドキュメント@<fn>{apollo-caching}を読みましょう。
このドキュメントに書いてあることは信じてよいと思います。
@<code>{cache.writeData}出てこないし…！

//footnote[apollo-caching][@<href>{https://www.apollographql.com/docs/react/advanced/caching.html}]

ハマりの説明に入りましょう。
筆者が作っていた（いる）アプリは、任意のデータベースの中身をdumpしてJSONとして取得する動作が含まれています。
任意のデータベースなので、もちろんスキーマは一定ではありません。
そのため、@<code>{scalar JSON}というcustom scalar typeを作成しました。
公式のドキュメント@<fn>{apollo-custom-scalar-type}にもこのやり方が（サーバ側に関してのみ）紹介されています。
うーん、なるほど？

//footnote[apollo-custom-scalar-type][@<href>{https://www.apollographql.com/docs/graphql-tools/scalars.html}]

更に調べていくとクライアント側のcustom scalar typeサポートはまだ存在していません@<fn>{apollo-client-side-custrom-scalar}。
サポートされていない、というのは値の形式変換が自動的にできないというだけで、実用上少し苦労する程度で済みます。

//footnote[apollo-client-side-custrom-scalar][@<href>{https://github.com/apollographql/apollo-feature-requests/issues/2}]

画面側の構成は、よくある左側にリストがあり、そこからアイテムを選択して右側で編集するという構成です。




2.5から本体に取り込まれたらしい。


cache.writeQueryとかcache.writeDataとかcache.writeFragmentとかマジでいろいろある…。
TODO それぞれの違いの解説

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

https://github.com/apollographql/apollo-client/issues/4554
https://www.apollographql.com/docs/react/advanced/caching.html
https://github.com/apollographql/react-apollo-error-template/issues/13

apollo-cache は実質1ファイル
https://github.com/apollographql/apollo-client/blob/c2295e6b85c5c016fcf935d2eb13d78f68a43541/packages/apollo-cache/src/cache.ts#L8
ApolloCacheはwrite.*系をwriteに変換したり、readも同様だったりで具体的な処理は少ない。
abstract methodを定めている。

apollo-cache-inmemoryは結構でっかい

https://www.npmjs.com/package/optimism

== 現時点でのベストプラクティス

TODO

== 日本におけるGraphQLコミュニティ（の不在）

TODO
