import { buildSchema, GraphQLSchema, graphql, ExecutionResult, IntrospectionQuery, print, printSchema, printIntrospectionSchema, buildClientSchema, visit, buildASTSchema, typeFromAST, GraphQLString, astFromValue, parse, TypeInfo, visitWithTypeInfo, graphqlSync, getIntrospectionQuery, introspectionQuery, DocumentNode } from "graphql";

describe("graphql", () => {
	// かなり色々あります。ここに載っていないのもの結構あります。
	// 使い勝手よさそうなものは網羅した気持ち…？

	it("buildSchema", () => {
		// #@@range_begin(buildSchema)
		// GraphQLのSchema Definition LanguageからGraphQLSchemaに変換する
		let schemaStr = `
			type Query {
				foo: String
			}
		`;
		const schema: GraphQLSchema = buildSchema(schemaStr);
		// #@@range_end(buildSchema)
		expect(schema).not.toBeNull();
		expect(schema).toMatchSnapshot();
	});
	it("printSchema", () => {
		// #@@range_begin(printSchema)
		// GraphQLSchemaからGraphQLのSchema Definition Languageに変換する
		let schemaStr1 = `
			type Query {
				foo: String
			}
		`;
		const schema: GraphQLSchema = buildSchema(schemaStr1);
		// schemaStr1と等価
		let schemaStr2: string = printSchema(schema);
		// #@@range_end(printSchema)
		expect(schemaStr2).toMatchSnapshot();
	});
	it("printIntrospectionSchema", () => {
		// GraphQLSchema(なんでもよさそう)からIntrospectionに組み込みの型のGraphQLのSchema Definition Languageに変換する
		let schemaStr = `
			type Query {
				foo: String
			}
		`;
		const schema: GraphQLSchema = buildSchema(schemaStr);
		let introspection: string = printIntrospectionSchema(schema);
		expect(introspection).toMatchSnapshot();
	});
	it("getIntrospectionQuery", async () => {
		// introspection queryの文字列
		// Queryとして投げればSchemaの型が取得できる
		// deprecated だった
		let schemaStr = `
			type Query {
				foo: String
			}
		`;
		const schema: GraphQLSchema = buildSchema(schemaStr);
		// #@@range_begin(getIntrospectionQuery)
		const introspectionQuery = getIntrospectionQuery();
		const result: ExecutionResult<IntrospectionQuery> =
			await graphql(schema, introspectionQuery);
		const schemaJSONlike = result.data;
		// #@@range_end(getIntrospectionQuery)
		expect(schemaJSONlike).not.toBeNull();
	});
	it("introspectionQuery", async () => {
		// introspection queryの文字列
		// Queryとして投げればSchemaの型が取得できる
		// deprecated だった
		expect(introspectionQuery).toMatchSnapshot();
		let schemaStr = `
			type Query {
				foo: String
			}
		`;
		const schema: GraphQLSchema = buildSchema(schemaStr);
		const result: ExecutionResult<IntrospectionQuery> = await graphql(schema, introspectionQuery);
		const schemaJSONlike = result.data;
		expect(schemaJSONlike).not.toBeNull();
	});
	it("graphql", async () => {
		// schemaに対してOperationを実行する
		// schemaはexecutable schemaである必要がある。はず
		let schemaStr = `
			type Query {
				foo: String
			}
		`;
		const schema: GraphQLSchema = buildSchema(schemaStr);
		// #@@range_begin(graphql)
		const result: ExecutionResult<IntrospectionQuery> =
			await graphql(schema, getIntrospectionQuery());
		// #@@range_end(graphql)
		expect(result).toMatchSnapshot();
		const schemaJSONlike = result.data;
		expect(schemaJSONlike).not.toBeNull();
	});
	it("graphqlSync", () => {
		// schemaに対してOperationを実行する
		// schemaはexecutable schemaである必要がある。はず
		let schemaStr = `
			type Query {
				foo: String
			}
		`;
		const schema: GraphQLSchema = buildSchema(schemaStr);
		const result: ExecutionResult<IntrospectionQuery> = graphqlSync(schema, getIntrospectionQuery());
		expect(result).toMatchSnapshot();
		const schemaJSONlike = result.data;
		expect(schemaJSONlike).not.toBeNull();
	});
	it("buildClientSchema", async () => {
		// GraphQLのSchema Definition LanguageからClient用のGraphQLSchemaに変換する
		// Client用のGraphQLSchemaはresolver, parse, serializeなどを持たない
		// …らしいが IntrospectionQueryとかは普通に処理できる。組み込みの機能だからかな？
		let schemaStr = `
			type Query {
				foo: String
			}
		`;
		const schema: GraphQLSchema = buildSchema(schemaStr);
		// #@@range_begin(buildClientSchema)
		const inspectionResult: ExecutionResult<IntrospectionQuery> =
			await graphql(schema, getIntrospectionQuery());
		const clientSchema: GraphQLSchema = buildClientSchema(inspectionResult.data!);
		// #@@range_end(buildClientSchema)
		expect(clientSchema).not.toBeNull();
		expect(clientSchema).toMatchSnapshot();

		{
			const inspectionResult: ExecutionResult<IntrospectionQuery> = await graphql(clientSchema, getIntrospectionQuery());
			expect(inspectionResult.data).not.toBeNull();
		}
	});
	it("parse", () => {
		// 文字列をDocumentNodeとしてparseする
		// graphql-tag とまぁだいたい一緒
		// #@@range_begin(parse)
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
		// #@@range_end(parse)
		expect(queryAST).toMatchSnapshot();
		expect(schemaAST).toMatchSnapshot();
	});
	it("print", () => {
		// ASTをprintする
		// schemaはprintできない（printSchemaを使おう）
		// #@@range_begin(print)
		let queryAST = parse(`
			query {
				foo: String
			}
		`);
		let queryStr: string = print(queryAST);
		// #@@range_end(print)
		expect(queryStr).not.toBeNull();
		expect(queryAST).toMatchSnapshot();
	});

	it("visit", () => {
		// ASTをvisitできます
		// 機能が色々書き方も色々なのでドキュメントを読みましょう
		// https://graphql.org/graphql-js/language/#visit
		// #@@range_begin(visit)
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
		// #@@range_end(visit)
		expect(count).toBe(6);
	});
	it("buildASTSchema", () => {
		// ASTからSchemaに変換する
		// #@@range_begin(buildASTSchema)
		let query = parse(`
			type Query {
				foo: String
			}
		`);
		const schema = buildASTSchema(query);
		// #@@range_end(buildASTSchema)
		expect(printSchema(schema)).toMatchSnapshot();
	});
	it("typeFromAST", () => {
		// Schemaから指定したTypeを拾ってくる…っぽい
		// 何に使うかイマイチ。validatorの実装とかで使われているっぽい。
		let schemaStr = `
			"""ドキュメント"""
			type Query {
				foo: String
			}
		`;
		const schema = buildSchema(schemaStr);
		const type = typeFromAST(schema, {
			kind: "NamedType",
			name: {
				kind: "Name",
				value: "Query",
			},
		});
		expect(type).toMatchSnapshot();
	});
	it("astFromValue", () => {
		// JSの値とGraphQLのtypeを渡すとGraphQLでの値のASTにして返してくれる
		const valueAST = astFromValue("test", GraphQLString);
		expect(valueAST).toMatchSnapshot();
	});
	it("TypeInfo", () => {
		// visitでASTをウロウロする時にその時点の型情報にアクセスできる
		// #@@range_begin(TypeInfo)
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
		// #@@range_end(TypeInfo)
	});
});
