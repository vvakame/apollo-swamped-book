= Apollo/GraphQLの素晴らしきユーティリティたち

#@# OK tomo: graphql と Apolloのライブラリが一緒に説明されているので"Apollo/graphql関連の" のほうがよいかもです。
#@# OK tomo:  2章の主旨はTypeScript用の型生成に役立つユーティリティの紹介ということですかね? 先に型生成のプロセスの概要(SDLとクエリからSchema.json生成し、それを元にcodegenなどで型定義情報を生成する)があり、そのフローの中で書くユーティリティがどう役に立って素晴らしいのか書くとフレンドリーかもです
#@# vv: あまり何も考えず、自分が知りたかったので調べて書きました…！

本章で紹介する関数を上手に組み合わせるとGraphQL SDLとIntrospectionQueryからapollo client:codegenに使うschema.jsonを生成したり、モックSchemaを作成したり、さまざまな恩恵を得ることができます。
特に、graphql@<fn>{npm-graphql}パッケージとgraphql-tools@<fn>{npm-graphql-tools}パッケージに含まれる便利な機能をピックアップして紹介します。
graphql-tag@<fn>{npm-graphql-tag}は使い方簡単だし…コードもすぐ読み切れるし割愛でいいかな！
#@# REVIEW OK sota1235: 重箱隅ですが、"ラク"より"恩恵"とかのほうがスンッと入ってくる気がします

//footnote[npm-graphql][@<href>{https://npmjs.com/package/graphql}]
//footnote[npm-graphql-tools][@<href>{https://npmjs.com/package/graphql-tools}]
//footnote[npm-graphql-tag][@<href>{https://npmjs.com/package/graphql-tag}]

== graphqlパッケージ

まずはgraphqlパッケージから紹介していきます。
graphqlパッケージはgraphql organizationからの提供で、GraphQLの仕様に対して参照実装という位置づけです。
このパッケージについて詳しく知りたい場合は、公式ドキュメント@<fn>{docs-graphql}を読むのがお勧めです。
ドキュメントに不足しがちなサンプルコードを本章で補っていきます。

//footnote[docs-graphql][@<href>{https://graphql.github.io/graphql-js/graphql/}]

AST関連やスキーマ関連の便利関数が揃っていて、メタな処理をしたい時に役立ちます。

特に断りなく未知の関数や型が出てきた場合、graphqlパッケージからimportしてきたものとします。
たとえば、@<code>{GraphQLSchema}と@<code>{buildSchema}が出てきた場合、@<code>{import { GraphQLSchema, buildSchema \} from "graphql";}というコードがあるものと考えてください。
@<code>{schema}は頻出のため、同名の変数が出てきた場合は@<code>{buildSchema}関数で作られた@<code>{GraphQLSchema}型とします。
#@# REVIEW OK sota1235: "特に断りのない場合は" => "特に断りなく"のほうが読みやすそうです

=== buildSchema関数

まずはbuildSchema関数です（@<list>{buildSchema}）。
GraphQL SDLからGraphQLSchemaに変換できます。

//list[buildSchema][buildSchema関数]{
#@maprange(../code/lib-examples/src/graphql.test.ts,buildSchema)
// GraphQLのSchema Definition LanguageからGraphQLSchemaに変換する
let schemaStr = `
  type Query {
    foo: String
  }
`;
const schema: GraphQLSchema = buildSchema(schemaStr);
#@end
//}

=== printSchema関数

これと対になるのがprintSchema関数です（@<list>{printSchema}）。
GraphQLSchemaからGraphQL SDLに変換できます。

//list[printSchema][printSchema関数]{
#@maprange(../code/lib-examples/src/graphql.test.ts,printSchema)
// GraphQLSchemaからGraphQLのSchema Definition Languageに変換する
let schemaStr1 = `
  type Query {
    foo: String
  }
`;
const schema: GraphQLSchema = buildSchema(schemaStr1);
// schemaStr1と等価
let schemaStr2: string = printSchema(schema);
#@end
//}

#@# REVIEW OK sota1235: 細かいですが最後の行の後ろにコメントで`schemaStr1と等価`みたいなコメントがあるとわかりやすいかもです

=== graphql関数

schemaに対してリクエストを作成し実行します（@<list>{graphql}）。
プロセス内で手軽にQueryなどを投げられるため使い勝手がよいです。
sync版であるgraphqlSyncもありますが、@<code>{GraphQL execution failed to complete synchronously.}とか言って怒られることがあるため、使えないと考えておいたほうが無難でしょう。

//list[graphql][graphql関数]{
#@maprange(../code/lib-examples/src/graphql.test.ts,graphql)
const result: ExecutionResult<IntrospectionQuery> =
  await graphql(schema, getIntrospectionQuery());
#@end
//}

=== getIntrospectionQuery関数

Introspection Queryに使う文字列を取得できるgetIntrospectionQuery関数です（@<list>{getIntrospectionQuery}）。
schema.jsonを生成する時に使われているQueryですね。
前述のgraphql関数と組み合わせてschema.jsonをひねり出すのにも使えます。

//list[getIntrospectionQuery][getIntrospectionQuery関数]{
#@maprange(../code/lib-examples/src/graphql.test.ts,getIntrospectionQuery)
const introspectionQuery = getIntrospectionQuery();
const result: ExecutionResult<IntrospectionQuery> =
  await graphql(schema, introspectionQuery);
const schemaJSONlike = result.data;
#@end
//}

=== buildClientSchema関数

schema.jsonからschemaを組み立てられるbuildClientSchema関数です（@<list>{buildClientSchema}）。
クライアント側でschemaがほしい場合、GraphQL SDLは持っていない場合が多いと思いますのでこちらを使うのが便利でしょう。

//list[buildClientSchema][buildClientSchema関数]{
#@maprange(../code/lib-examples/src/graphql.test.ts,buildClientSchema)
const inspectionResult: ExecutionResult<IntrospectionQuery> =
  await graphql(schema, getIntrospectionQuery());
const clientSchema: GraphQLSchema = buildClientSchema(inspectionResult.data!);
#@end
//}

=== parse関数

渡した文字列をパースしASTにして返します（@<list>{parse}）。
必要になる場面はbuildSchemaなどに比べると少ないかもしれないですね。

//list[parse][parse関数]{
#@maprange(../code/lib-examples/src/graphql.test.ts,parse)
let queryAST: DocumentNode = parse(`
  query {
    foo
  }
`);
let schemaAST: DocumentNode = parse(`
  type Query {
    foo: String
  }
`);
#@end
//}

ちなみに、grapgl-tagが返す値の実体はparse関数と同じくDocumentNodeです。
#@# OK gfx: 実体は DocumentNode だけど .d.ts での宣言はanyになってるんでな…
#@# https://github.com/apollographql/graphql-tag/pull/141 一度は型が与えられたけど問題があってrevertされたっぽい
#@# vv: コード読んでanyになってるけど実際DocumentNodeだなーというとこまでは追ってたんだけど過去にチャレンジされたことがあったのか… てっきり怠慢かと思(ｒｙ

=== print関数

parseとは逆に、ASTを文字列に変換します（@<list>{print}）。
必要に応じてprintSchemaとprintを使い分けるとよいでしょう。

//list[print][print関数]{
#@maprange(../code/lib-examples/src/graphql.test.ts,print)
let queryAST = parse(`
  query {
    foo: String
  }
`);
let queryStr: string = print(queryAST);
#@end
//}

=== buildASTSchema関数

（スキーマの）ASTからSchemaに変換します。（@<list>{buildASTSchema}）。
これも使うタイミングは少なさそうです。

//list[buildASTSchema][buildASTSchema関数]{
#@maprange(../code/lib-examples/src/graphql.test.ts,buildASTSchema)
let query = parse(`
  type Query {
    foo: String
  }
`);
const schema = buildASTSchema(query);
#@end
//}

=== visit関数とTypeInfoクラス

ASTを深さ優先探索でトラバースします（@<list>{visit}）。
visitorの書き方はかなり色々あり、Nodeの差し替えなども可能なので、詳しくは公式ドキュメント@<fn>{docs-visit}を読んでみてください。

//footnote[docs-visit][@<href>{https://graphql.org/graphql-js/language/#visit}]

//list[visit][visit関数]{
#@maprange(../code/lib-examples/src/graphql.test.ts,visit)
let query = parse(`
  query {
    foo: String
  }
`);
let count = 0;
visit(query, {
  enter(node) {
    count++;
    expect(node).not.toBeNull();
  }
});
#@end
//}

visitorで参照しているnodeの型を知りたい場合があります。
そのときはTypeInfoのインスタンスを作成し、visitWithTypeInfo関数を併用することで型情報を得ることができます。

//list[TypeInfo][TypeInfoを併用して型を得る]{
#@maprange(../code/lib-examples/src/graphql.test.ts,TypeInfo)
let schemaStr = `
  type Query {
    foo(id: ID): String
  }
`;
const schema: GraphQLSchema = buildSchema(schemaStr);
let typeInfo = new TypeInfo(schema);
 let query = parse(`
  query {
    foo(id: "a")
  }
`);
visit(query, visitWithTypeInfo(typeInfo, {
  enter(node) {
    const parentType = typeInfo.getParentType();
    const parentInputType = typeInfo.getParentInputType();
    const type = typeInfo.getType();
    const inputType = typeInfo.getInputType();
    expect({
      nodeKind: node.kind,
      nodeName: node.kind === "Name" ? node.value : null,
      parentType,
      parentInputType,
      type,
      inputType,
    }).toMatchSnapshot();
  }
}));
#@end
//}

== graphql-toolsパッケージ

次はgraphql-toolsパッケージ@<fn>{docs-graphql-tools}です。
こちらはapollographql organization配下で管理されている点に注意してください。

//footnote[docs-graphql-tools][@<href>{https://www.apollographql.com/docs/graphql-tools/}]

こちらにもいくつか有用なユーティリティがあるので抜粋して紹介します。

特に断りなく未知の関数や型が出てきた場合、graphql-toolsパッケージからimportしてきたものとします。
graphql関数についてはgraphqlパッケージからimportしてきたものとします。
#@# REVIEW OK sota1235: 27行目と表記ゆれしてます！統一してあげるとよさそう

=== makeExecutableSchema関数

GraphQLサーバと同等の動作をさせるために必要な、不足しているResolverなどの要素をSchemaに組み込みます（@<list>{makeExecutableSchema}）。
クライアント側の実装を行う時にはあまり使わない…かと思いきや、Apolloのドキュメントを参考にコードを書いていると案外出会います。
#@# REVIEW OK sota1235: 実際に"何を"動作させるのかぱっとわかりづらいので、主語を入れてあげるとよさそうです(GraphQLサーバ、かな？)

//list[makeExecutableSchema][makeExecutableSchema関数]{
#@maprange(../code/lib-examples/src/graphql-tools.test.ts,makeExecutableSchema)
// typeDefs に使える型はかなり幅広い
// resolvers の型も色々
const typeDefs = `
  type Query {
    foo: String
  }
`;
const resolvers = {
  Query: {
    foo: () => "foo!",
  },
};
 const schema = makeExecutableSchema({
  typeDefs,
  resolvers,
});
 const result = await graphql(schema, `{ foo }`);
#@end
//}

=== addMockFunctionsToSchema関数

schemaに対してmockのresolverをセットしてくれます（@<list>{addMockFunctionsToSchema}）。
テストを書く時や、とりあえず動く何かがほしい時に重宝します。
Mockに使う関数やらリストやらをカスタマイズすることもできます@<fn>{docs-mocking}。

//footnote[docs-mocking][@<href>{https://www.apollographql.com/docs/graphql-tools/mocking}]

//list[addMockFunctionsToSchema][addMockFunctionsToSchema関数]{
#@maprange(../code/lib-examples/src/graphql-tools.test.ts,addMockFunctionsToSchema)
const typeDefs = `
  type Query {
    foo: String
    bar: Int
  }
`;
const resolvers = {
  Query: {
    foo: () => "foo!",
  },
};
 const schema = makeExecutableSchema({
  typeDefs,
  resolvers,
});
// schemaのresolversに適当な値を詰めて返してくれるモック関数をセットしてくれる
addMockFunctionsToSchema({
  schema,
  mocks: {
    Int: () => 42,
  },
  preserveResolvers: true,
});
 const result = await graphql(schema, `{ foo bar }`);
#@end
//}

#@# REVIEW sota1235: 一番最初のコメントは`addMockFunctionsToSchema({`の手前に合ったほうがよさそう

buildClientSchemaとaddMockFunctionsToSchemaを組み合わせると、クライント側の手持ちのパーツから簡単にMockを作ることができます。
テスト用のお手軽サーバとして活用すると便利です。

//comment{
 * makeExecutableSchema
 * addResolveFunctionsToSchema
 * addSchemaLevelResolveFunction
 * addMockFunctionsToSchema
 ** buildClientSchema と addMockFunctionsToSchema の組み合わせなるほどなー
 * mockServer
 * SchemaDirectiveVisitor（サーバ側実装に必要なのでまぁいらないかな？）
 * makeExecutableSchema 同上
 * mergeSchemas
 * graphql-resolvers パッケージ
//}
