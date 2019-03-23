= 小ハマり集

== QueryとfetchMoreの謎

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
公式のPaginationの説明@<fn>{apollo-pagenation}には必要なことが書いてあって、ここでやっていることを丸コピすればだいたいOKです。
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

== 現時点でのベストプラクティス

TODO

== 日本におけるGraphQLコミュニティ（の不在）

TODO

== Subscriptionを何かしらのPushとQueryに変換する

TODO

== react-apolloでタグに固執せずApolloConsumerを使う話

TODO Mutation とかにこだわってると自由度低い…みたいな話

== FragmentがFragmentを使う場合にいい感じに@clientが消してくれない

TODO

== Apolloでコードにドキュメントを書かないのでだるい話

TODO
