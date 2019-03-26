= graphql関連の素晴らしきユーティリティたち

GraphQLはかなり潤沢なユーティリティがあり、これらを自由に活用することができます。
ここでは、graphql@<fn>{npm-graphql}とgraphql-tools@<fn>{npm-graphql-tools}について便利そうな機能をピックアップして紹介します。
graphql-tag@<fn>{npm-graphql-tag}は使い方簡単だし…コードもすぐ読めるしまぁいいか！

//footnote[npm-graphql][@<href>{https://npmjs.com/package/graphql}]
//footnote[npm-graphql-tools][@<href>{https://npmjs.com/package/graphql-tools}]
//footnote[npm-graphql-tag][@<href>{https://npmjs.com/package/graphql-tag}]

== graphqlパッケージ

まずはgraphqlパッケージから紹介していきます。
公式ドキュメント@<fn>{docs-graphql}を読むのがおすすめです。
本章のサンプルコードも合わせて読むとよりわかりやすいでしょう。

//footnote[docs-graphql][@<href>{https://graphql.github.io/graphql-js/graphql/}]

AST関連やスキーマ関連の便利関数が揃っていて、メタな処理をしたい時に役立ちます。

特に断りのない場合、未知の関数や型が出てきた場合、graphqlパッケージからimportしてきたものとします。
たとえば、GraphQLSchemaとbuildSchemaが出てきた場合、@<code>{import { GraphQLSchema, buildSchema \} from "graphql";}というコードがあるものと考えてください。
GraphQLSchema型の値であるschemaは頻出のため、これについてはbuildSchema関数などを使って作成しているものとします。

=== buildSchema

まずはbuildSchema関数です（@<list>{buildSchema}）。
GraphQL Schema Definition LanguageからGraphQLSchemaに変換できます。

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

=== printSchema

これと対になるのがprintSchema関数です（@<list>{printSchema}）。
GraphQLSchemaからGraphQL Schema Definition Languageに変換できます。

//list[printSchema][printSchema関数]{
#@maprange(../code/lib-examples/src/graphql.test.ts,printSchema)
// GraphQLSchemaからGraphQLのSchema Definition Languageに変換する
let schemaStr1 = `
  type Query {
    foo: String
  }
`;
const schema: GraphQLSchema = buildSchema(schemaStr1);
let schemaStr2: string = printSchema(schema);
#@end
//}

=== graphql

schemaに対してリクエストを作成し実行します（@<list>{graphql}）。
クライアント側だけでざっくりクエリを投げられるため使い勝手がよいです。
sync版であるgraphqlSyncもありますが、GraphQL execution failed to complete synchronously.とか言って怒られることがあるため、使えないと考えておいたほうが無難でしょう。

//list[graphql][graphql関数]{
#@maprange(../code/lib-examples/src/graphql.test.ts,graphql)
const result: ExecutionResult<IntrospectionQuery> =
  await graphql(schema, getIntrospectionQuery());
#@end
//}

=== getIntrospectionQuery

IntrospectionQueryに使う文字列を取得できるgetIntrospectionQuery関数です（@<list>{getIntrospectionQuery}）。
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

=== buildClientSchema

schema.jsonからschemaを組み立てられるbuildClientSchema関数です（@<list>{buildClientSchema}）。
クライアント側でschemaがほしい場合、GraphQL SDLは持っていない場合が多いと思いますのでこちらを使うのが便利でしょう。

//list[buildClientSchema][buildClientSchema関数]{
#@maprange(../code/lib-examples/src/graphql.test.ts,buildClientSchema)
const inspectionResult: ExecutionResult<IntrospectionQuery> = await graphql(schema, getIntrospectionQuery());
const clientSchema: GraphQLSchema = buildClientSchema(inspectionResult.data!);
#@end
//}

=== parse

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

=== print

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

=== buildASTSchema

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

=== visitとTypeInfo

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
その時はTypeInfoのインスタンスを作成し、visitWithTypeInfo関数を併用することで型情報を得ることができます。

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
こちらはapollographql organization配下で管理されています。

//footnote[docs-graphql-tools][@<href>{https://www.apollographql.com/docs/graphql-tools/}]

こちらにもいくつか有用なユーティリティがあるので抜粋して紹介します。

特に断りのない場合、未知の関数や型が出てきた場合、graphql-toolsパッケージからimportしてきたものとします。
graphql関数についてはgraphqlパッケージからimportしてきたものとします。

=== makeExecutableSchema

Resolverなど、実際に動作させるために必要な要素を組み込んだSchemaを作成します（@<list>{makeExecutableSchema}）。
クライアント側の実装を行う時にはあまり使わない…かと思いきや、Apolloのドキュメントを参考にコードを書いていると案外出会います。

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

=== addMockFunctionsToSchema

schemaに対してmockのresolverをセットしてくれます（@<list>{addMockFunctionsToSchema}）。
テストを書く時や、とりあえず動く何かがほしい時に重宝します。
Mockに使う関数やらリストやらをカスタマイズすることもできます@<fn>{docs-mocking}。

//footnote[docs-mocking][@<href>{https://www.apollographql.com/docs/graphql-tools/mocking}]

//list[addMockFunctionsToSchema][addMockFunctionsToSchema関数]{
#@maprange(../code/lib-examples/src/graphql-tools.test.ts,addMockFunctionsToSchema)
// schemaのresolversに適当な値を詰めて返してくれるモック関数をセットしてくれる
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