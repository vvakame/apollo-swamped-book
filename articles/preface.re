= はじめに

技術書典5ではGraphQLサーバをGo言語で作るというネタをやりました。
今回は、所変わってクライアント側を普通にReact+TypeScript+Apolloでクライアント側をやっていきつつある話をします。

本書の内容は次のバージョンを前提に解説しています@fn<monorepo-versioning>。

//footnote[monorepo-versioning][なんでmono repoなのにバージョンがちぐはぐなんだ…？]

 * apollo@2.6.2
 * apollo-client@2.5.1
 * apollo-link@1.2.11
 * apollo-cache-inmemory@1.5.1

GraphQLの概要についてはそろそろ割愛していいと思いますので割愛します。
Google先生で調べるか、私による解説が読みたい人は@<href>{https://vvakame.booth.pm/items/1055228}を参照してください。


さて、今回のメインのネタは"Apollo泣き言集"です。
筆者が陥ったドハマリについて解説していきたいと思います。
皆さんは私のような思いはしてほしくない…俺の屍を越えてゆけ…！
デバッグログとかもうちょっとなんか出力してくれないですかね…。

公式ドキュメントは若干長ったらしく、そのくせ網羅的な解説ではありません。
よって、使っていい仕様と使わないほうがよい仕様がわかりにくいです。
掲載されているサンプルコードも抜粋が多く、全体像がわかりにくく、動作させられるサンプルがあるのかもわかりにくいです（だいたい無い印象）。

さらに、Apolloはサーバ側もカバーしているため、サーバ側の話とクライアント側の話を区別する必要があります。
特定のトピックに関して、サーバ側でのやり方は書いてあってもそれに対応するクライアント側のやり方が併記されていないことがほとんどです。
さらに、基本的にはJavaScriptの話題しか見当たりません。
ApolloはJavaやScalaやSwiftもサポートしているようなのですが、それらをやっていきたい人はさらに獣道を歩くことになるのではないでしょうか。

総じて、やりたいことをパッと実現できなかった場合、コードを読みデバッガをお供に冒険する必要があります。
