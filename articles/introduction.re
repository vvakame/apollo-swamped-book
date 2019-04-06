= クライアント側でApolloやっていき

最近は技術書典Webで使おうとしたり、社内ツールをフルスクラッチする機会を利用してApollo@<fn>{why-apollo}の素振りをしています。
Apolloは大枠としてはわかりやすく、第一歩で躓くことは少ないように思います。
この章ではApolloがもたらす恩恵について書いていきます。

//footnote[why-apollo][そもそもなぜApolloなのか？という話ですが、わりと消極的で他にいいのがないからです。素敵な移行先を常に募集しています。]

#@# OK tomo: 1章にApollo使うメリットな説明があるとよいかもと思いました。具体的には、1.1の"型があるってすばらしい"はGraphQL単体でも実現できる(例えば https://github.com/prisma/graphqlgen など)のでApolloのメリットというわけではない気がします。GraphQLのライブラリはRelayも有名ですが、それよりApolloを使うメリットはGraphQLを扱いやすくするためのツール群が充実していること、ReactやVueなどのフレームワークに沿ったライブラリがあることと思うので、その点のメリットを説明した上で、2章以降でそのツールの紹介などにつなげていくとよさそうです!
#@# vv: Apolloを選んだ動機を説明するのが結構難しい…
#@# vv: prismaはクライアント側ライブラリではない…？（あまりよく調べてない）
#@# vv: Realyを選ばなかったのはFb製なのでFlow文化なのと、TypeScriptサポートきたけどFbはOSSの運用上手くない気がする… みたいな消極的回避が働いています
#@# vv: https://github.com/vvakame/til/tree/master/graphql/relay-examples-to-typescript
#@# vv: 消去法でApollo… というわりと消極的な理由です。

== 型があるって素晴らしい

なんといってもGraphQLは型を持っていることが最重要ポイントです。
型があるなら、静的にクエリが正しいか否か判定することができますし、クエリのレスポンスデータの"完全な"型を得ることもできます。

GitHubが提供するv4 API@<fn>{github-v4}はGraphQLで提供されていますので、これを例として見せます。
アプリを組む時の大まかな流れは次のようになります。

//footnote[github-v4][@<href>{https://developer.github.com/v4/explorer/}]

 1. GraphQLのスキーマのデータをどこかからひねり出す
 2. クエリを書く
 3. クエリから型定義ファイルを生成する

まずは1つ目のステップです。
GitHubはAPIを叩く時にAccessTokenを必要としますので、Personal Access Token@<fn>{github-pat}などを適当に準備します。

//footnote[github-pat][@<href>{https://github.com/settings/tokens}]

//cmd{
$ npx apollo schema:download --endpoint https://api.github.com/graphql \
    --header "Authorization: Bearer ${GITHUB_TOKEN}"
  ✓ Loading Apollo Project
  ✓ Saving schema to schema.json
//}

これでGitHubのGraphQLの型情報が詰まったschema.jsonが作成されました。

次にクエリを書いてみます（@<list>{code/github-query/src/index.ts}）。

//list[code/github-query/src/index.ts][自分のIDとbioを取得するクエリ]{
#@mapfile(../code/github-query/src/index.ts)
import gql from "graphql-tag";

const viewerQuery = gql`
  query ViewerQuery {
    viewer {
      # 必要な分だけ書けばいい！
      id
      bio
      here:location
    }
  }
`;
#@end
//}

クエリを@<code>{graphql-tag}に文字列テンプレートリテラルとして渡してやるだけです。
他のFragmentの参照も@<code>{${otherFragmentDefinition\}}という感じに自然に行うことができます。

QueryやFragmentにはしっかりとアプリケーション全体でユニークな名前をつけましょう。
grapqhl-tagパッケージは名前を元にキャッシュをしようとしますし、apollo client:codegenによるTypeScriptの型定義の生成も名前無しだとエラーになります。
この習慣はサーバ側でトレースを分析する時にも便利です。
なんらかの命名規則を設け、しっかり管理するのをお勧めします。
#@# OK gfx: 名前つけなくても実行はできる気がするぞ？？とはいえ名前をつるのがベストプラクティスなのは確かです。あと、fagment nameはアプリ全体でユニークでないといけないです（graphql-tagの仕様な気もするが、graphql-tagもapolloの一部なので）
#@# cf. https://github.com/apollographql/graphql-tag/blob/master/src/index.js#L45-L51
#@# vv: apollo client:codegen で怒られるので必須だと思ってた…この世に型のない世界線なんて存在したのか…

ここのソースコードをapollo先生に解析してもらい、型定義情報を出力してもらいます。

//cmd{
$ npx apollo client:codegen --localSchemaFile=./schema.json \
    --addTypename --target=typescript --outputFlat src/graphql

  ✓ Loading Apollo Project
  ✓ Generating query files with 'typescript' target - wrote 2 files
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
また、nullableかどうかもキチンと処理され、nullableかどうかも型として表現されています。

このように、GraphQL+Apollo+TypeScriptではアプリケーション的に関心がある型を厳密に扱えるようになっています。
#@# REVIEW sota1235: "アプリケーション的に関心がある" => 少しピンと来ないかも(意味は分かる)、"アプリケーションで使用する型のみを厳密に~"とかのほうが言いたいことと近いのかなぁと思いました
ワンダフル！
Open API（Swagger）も似たようなことができます。
#@# REVIEW sota1235: 上の文と下の文をつなげたほうがわかりやすいです。"...も似たようなことができますが、アプリケーション側の関心と..."
しかし、アプリケーション側の関心とサーバ側が提供する型が厳密に一致することがほぼないのが不便です。
#@# OK gfx: s/GraphQL+apollo/GraphQL+Apollo+TypeScript/

===[column] Apolloで扱うスキーマあれこれ

GraphQLの型情報を持ったファイル、と言った時、該当するものが複数あります。

1つ目が主に拡張子.graphqlで表されるファイルで、中身は人間が読み書きするためのスキーマ定義です（@<list>{graphql-ext-example}）。
@<kw>{GraphQL SDL,GraphQL Schema Definition Language}と呼ばれる場合もあります。
主な使われ方として、.graphqlは人間が読み書きし、サーバ側実装のソースにします。

//list[graphql-ext-example][*.graphqlの例]{
type Query {
  node(id: ID!): Node
  nodes(ids: [ID!]!): [Node]!
}
//}

2つ目が、schema.jsonなどで表される型情報が集まったJSONファイルです。
このJSONファイルはサーバ側のGraphQLのエンドポイントを叩き、Introspectionの機能を使って型情報を取得します。
多くはGraphQL SDL→サーバ→schema.jsonというフローです。

*.graphqlからサーバを経由することなしにschema.jsonにする方法もあります。
筆者も駆け出しの頃にやり方を探したけどなかなか見つからず苦労したので、今使っているスクリプトをgistにあげておきます。

@<href>{https://gist.github.com/vvakame/0d92c9101e6db6fa6f5f2ab714bca00e}

型定義や型情報と言った時、この2つのうちどっちを指すのかは文脈やツールに依存します。
気合でわかってください。

3つ目が、GraphQL SDLをパースした結果のASTです。
基本的にお目にかかることは少ないでしょう。

===[/column]

== Apolloという魔法

GraphQLのクエリを書くと型定義が生成される！というのはかなり魔法っぽいです。
自分で実装しろといわれたら、アプリケーションコードのASTを見て、特定のパターンをクエリとして処理して…という流れが想像できます。
ある種魔法的な実装が必要になるわけですが、このApolloの提供する魔法の力は一体どんなものなのでしょうか？
我々の想像よりも強いところも弱いところもあります。

まずは筆者がすごい！と思っていることをリストアップしてみます。

 * コード中のQueryやらMutationを拾い上げて型定義を出力してくれる
 * ローカルの状態管理もできる（Redux不要説）
 * リモートとローカルへのクエリを混ぜて書ける
 ** 自動的に分解・統合を裏でやってくれる
 * データのキャッシュを賢く管理してくれる
 ** データの更新が検知できたら、関連箇所に自動的に反映される

素晴らしい！

一方、不満がある点は次のとおり。

 * リストの継ぎ足し（fetchMore）が思ったより自動じゃない
 * ちょいちょい謎のワークアラウンドがありお気持ちを忖度するのが難しい

おつらい…。

現時点の筆者の感想として、Apolloは"現実に存在する"ユースケースを広くカバーしようと頑張っています。
そのため、"あの仕様を踏襲していればこういう魔法がかかるのでは…？"という期待は打ち砕かれることが多いです。
これについては後続の章でおいおい取り上げていきます。

Apolloをやっていく上で、Relay global object identification@<fn>{relay-global-obj-id}の存在はしっかり抑えておいたほうがよいです。
GraphQLにおいては、すべての非スカラなオブジェクトはグローバルなIDで識別できるのがよいです。
力強くやっていきましょう。
筆者は普通に@<code>{テーブル名:PK}ぐらいのノリでIDを作ってます。

//footnote[relay-global-obj-id][@<href>{https://facebook.github.io/relay/graphql/objectidentification.htm}]

//comment{
TODO
 * http://lightbulbcat.hatenablog.com/entry/2018/02/18/000135 に載ってた表が有益な話
//}
