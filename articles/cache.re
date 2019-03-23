= キャッシュでドハマリ

本章のTL;DR

 * @<code>{cache.writeData}は極力使うな
 * IDを定めにくいデータをキャッシュに書くには@<code>{cache.writeQuery}を使え
 * IDが決められるデータをキャッシュに書くには@<code>{cache.writeFragment}を使え
 * Apolloでドハマリしそうになったらコード読んでデバッガ使って処理を追いかけろ

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

ハマりの説明に入りましょう。
筆者が作っていた（いる）アプリは、任意のデータベースの中身をdumpしてJSONとして取得する動作が含まれています。
任意のデータベースなので、もちろんスキーマは一定ではありません。
そのため、@<code>{scalar JSON}というcustom scalar typeを作成しました。
公式のドキュメント@<fn>{apollo-custom-scalar-type}にもこのやり方が（サーバ側に関してのみ）紹介されています。
うーん、なるほど？

//footnote[apollo-custom-scalar-type][@<href>{https://www.apollographql.com/docs/graphql-tools/scalars.html}]

更に調べていくとクライアント側のcustom scalar typeサポートはまだ存在していません@<fn>{apollo-client-side-custrom-scalar}。
サポートされていない、というのは値の形式変換が暗黙理にできないというだけで、実用上少し苦労する程度で済みます。

//footnote[apollo-client-side-custrom-scalar][@<href>{https://github.com/apollographql/apollo-feature-requests/issues/2}]

画面側の構成は、よくある左側にリストがあり、そこからアイテムを選択して右側で編集するという構成です。
選択し編集する操作するため、選んだアイテムの情報をどこかに保持しておかなければなりません。
これを実現するため、@<code>{@client}をつけたmutationで"現在編集中のアイテム"を状態として保持します。

#@# TODO サンプルコードをこの辺に…

この時、"現在編集中のアイテム"に@<code>{scalar JSON}なデータが含まれていた場合、問題が顕在化します。
@<code>{cache.writeData}はスキーマレスなデータ@<strong>{しか}渡すことができません。
また、apollo-clientは実行時にはスキーマに関する情報を持っていません。
ここはかなり盲点ですね。
Issueも書いてみた@<fn>{apollo-issue-4554}のですが、反応まるでないし@<code>{cache.writeQuery}か@<code>{cache.writeFragment}使え、で終わりそうではあります。
#@# NOTE ROOT_QUERY にwriteFragment経由で書き込もうとする

//footnote[apollo-issue-4554][@<href>{https://github.com/apollographql/apollo-client/issues/4554} まぁ筋の良い話ではなさそう]

さて、キャッシュのデータは必ずスキーマとセットで扱われます。
@<code>{cache.writeData}では、このギャップを埋めるために@<code>{queryFromPojo}というユーティリティ関数を使って、データからスキーマをひねり出して使います。
しかしながら、当然データのどの部分が@<code>{scalar JSON}なのかがわからないため、JSONの中まで見に行って@<code>{__typename}とか@<code>{id}がなくて大騒ぎになったりします。
この挙動はドキュメントには明記されていないように見えますし、ひたすらデバッガ片手に処理を追いかけていかないと原因がわかりませんでした。
最終的に、設計思想と自分の実装にミスマッチがありうまく行っていないことがわかるまで非常に苦労しました。
壊れたデータを書いてしまった場合でも怒られず、挙動はしっかりとおかしくなるし、一見関係ないところでQueryの結果がおかしくなるみたいな挙動になるので本当に罠みが高いです。

@<code>{cache.writeData}は罠！GraphQLでコードを書くというのにスキーマレスになりうるパーツを使うのは罪！
そのことを強く胸に刻みこんで生きていきましょう。

どういう方向性で実装するのがよかったかを解説します。
@<code>{cache.writeQuery}か@<code>{cache.writeFragment}を常に使うのが正しいです。
QueryかFragmentを与えると、書き込みたいデータに対してスキーマとして機能し、それに沿ってデータ形式が検査されます。
つまり、QueryやFragmentをバリデータとして与えるという意味合いになります。
QueryやFragmentは@<code>{apollo client:codegen}を使うことで間接的にスキーマに対して正しい形式であることを静的にチェックできます。

ここでいうQueryやFragmentは、Apolloに何かをリクエストした時の実際にサーバに送られるQueryやFragmentと同一のものではありません。
apollo-clientの中にキャッシュは常に1つであり、そこに対する書き込み、読み込みの型としてQueryやFragmentを流用しているに過ぎません。

2つの使い方として、IDが判明している単一のデータの書き込みには@<code>{cache.writeFragment}を使い、そうでなければ@<code>{cache.writeQuery}を使うことになります。
とりあえず@<code>{cache.writeFragment}を使って、それが無理な場合は@<code>{cache.writeQuery}を使う、と覚えておくとよいでしょう。

#@# TODO writeFragmentの使い方とかを乗せる

試行錯誤する過程で、コレ以外にもいくつかプラクティスを発見したので紹介していきます。

clientのresolversより先に、cacheのcacheRedirectsで解決できないか検討する。
発想として、クライアントローカルな処理を定義する時にclientのresolversに実装を追加した解決したくなります。
しかし、clientとcacheはレイヤーが分かれていて、アプリケーション→client→cacheと処理が流れていくことを忘れてはいけません。
cacheのレイヤーで無理なく実装できることはcacheのレイヤーで実装するのがよいでしょう。

#@# TODO この辺もうちょっと具体的な理由があった気がするけど忘れた

ローカルに状態を持つ時、可能な限り独自のtypeやらを定義しない。
単純に@<code>{__typename}やら@<code>{id}やら、そのルールを定義しメンテするのがめんどくさいからです。
そして、キャッシュの仕組み上@<code>{__typename}と@<code>{id}が定義できないような構造を導入すると痛い目を見ることになります。
初期状態を定義するときに適当なデフォルト値をセットして回るのが面倒…というのもあるのですが、その苦労はこなしておくのが妥当です。
現状、筆者はサーバ側もクライアント側も自分で書いているのでシステム全体の一貫性を考慮して設計するのは比較的カンタンなのです。
しかし、大規模化した時のことを考えると余計な複雑さは抑えておくべきでしょう。
筆者も最初はQuery配下に色々な要素を散らかすのはちょっと…と思い、ローカルの状態を管理する型を作ろうかと思いました。
しかし、すぐに@<code>{id}が定義できないので@<code>{cache.writeFragment}が使えず面倒になってしまいました。
代わりに、Queryの直下に@<code>{current}プリフィクスをつけたフィールドを定義し、ローカルの状態管理にあてています。

#@# TODO writeQueryで書き込むとネストした箇所の部分的な更新ができずにつらい目にあう気がするので検証しましょうね

@<code>{@export}を利用して頑張ろうとしない。
GraphQLはクエリであるため、関数を持ちません。
これがかなり厳しい制約で、@<code>{ID}を@<code>{Boolean}に変換したりすることはできません。
つまり、値の有無や特定の値の時に@<code>{@skip}や@<code>{@include}を使うことができません。
変数の値の変換をどこかのレイヤーでできると良いのでしょうが、筆者の知るかぎり難しいようです。
代わりにたどり着いたのがcacheのcacheRedirectsを使う方法だった、というわけです。

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

皆さんにおすすめしたいのが、とりあえずハマる前にキャッシュ関係のソースコードを読め！ということです。
apollo-cacheの実装@<fn>{apollo-cache-src}を読んでみて驚くのは、そのコード量の少なさです。
めっちゃサクサク読める！
前述の@<code>{writeData}の実装や、@<code>{writeQuery}、@<code>{writeFragment}がどういう挙動をしようとしているのか把握しておくとドハマリが避けられます。

//footnote[apollo-cache-src][@<href>{https://github.com/apollographql/apollo-client/tree/61639bcf44981a879f20c6196f74a7f7244bfda4/packages/apollo-cache}]

apollo-cache-inmemoryの実装@<fn>{apollo-cache-inmemory-src}も読むとタメになりますが、優先度は低いでしょう。
ドキュメント類の網羅度が貧弱なので謎メソッドの実装を読んで振る舞いを把握する必要があります。

//footnote[apollo-cache-inmemory-src][@<href>{https://github.com/apollographql/apollo-client/tree/61639bcf44981a879f20c6196f74a7f7244bfda4/packages/apollo-cache-inmemory}]

apollo-cache-inmemoryはoptimismパッケージ@<fn>{npm-optimism}に依存しているのですが、こいつが複雑&ドキュメント皆無というめんどくさ奴です。
めんどくさいので読むのを途中で諦めたのですが、デバッガで追いかけている時にapolloのコードにいるのかoptimismのコードにいるのかが判断できる程度に様子を把握しておくと便利です。

//footnote[npm-optimism][@<href>{https://www.npmjs.com/package/optimism}]

なぜドキュメントが全然ないんだお前らァ…！

//comment{
https://github.com/apollographql/react-apollo-error-template/issues/13
//}