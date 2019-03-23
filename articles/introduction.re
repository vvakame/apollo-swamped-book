= クライアント側でApolloやっていき

最近は技術書典Webで使おうとしたり、あと社内ツールをフルスクラッチする機会を手に入れたので素振りがてら色々やってみています。
Apolloは大枠としてはわかりやすく、第一歩で躓くことは少ないように思います。
この章ではApolloがもたらす恩恵について書いていきます。

== 型があるって素晴らしいよな

なんといってもGraphQLは型を持っていることが最重要ポイントです。
型があるということは、静的にクエリがvalidか否か判定することができますし、クエリで得られるデータに合致する"完全な"型を得ることができます。

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
Fragmentの埋込なども@<code>{${otherFragmentDefinition\}}的に行うことができます。

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
Open API（Swagger）も似たようなことができますが、アプリケーションの需要とサーバ側が出力する型が厳密には一致しないのがちょっと不便です。

===[column] Apolloで扱うスキーマあれこれ

GraphQLの型情報を持ったファイル、と言った時、該当するものが複数あります。

1つ目が主に拡張子.graphqlで表されるファイルで、中身は人間が読み書きするためのスキーマ定義です（@<list>{graphql-ext-example}）。

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

まずは筆者がすごい！と思っていることをリストアップしてみます。

 * コード中のQueryやらMutationを拾い上げて型定義を出力してくれる
 * ローカルの状態管理もできる（Redux不要説）
 * リモートとローカルへのクエリを混ぜて書ける
 ** 自動的に分解・統合を裏でやってくれる
 * キャッシュを賢く行ってくれる
 ** データの更新がかかると関連箇所に自動反映される

素晴らしい！
一方、不満がある点は次の通り。

 * リストの継ぎ足し（fetchMore）が思ったより自動じゃない
 * ちょいちょい謎のワークアラウンドがありお気持ちを忖度するのが難しい

おつらい…。

現時点の筆者の感想として、Apolloは"現実に存在する"ユースケースを広くカバーしようと頑張っています。
そのため、"あの仕様を踏襲していればこういう魔法がかかるのでは…？"という期待は打ち砕かれることが多いです。
これについては後続の章でおいおい取り上げていきます。

Apolloをやっていく上で、Relay global object identification@<fn>{relay-global-obj-id}の存在はしっかり抑えておいたほうがよいです。
GraphQLにおいては、全ての非スカラなオブジェクトはグローバルなIDで識別できるのがよいです。
力強くやっていきましょう。
筆者は普通に@<code>{テーブル名:PK}ぐらいのノリでIDを作ってます。

//footnote[relay-global-obj-id][@<href>{https://facebook.github.io/relay/graphql/objectidentification.htm}]

//comment{
TODO
 * http://lightbulbcat.hatenablog.com/entry/2018/02/18/000135 に載ってた表が有益な話
//}
