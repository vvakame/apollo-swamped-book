import { makeExecutableSchema, addCatchUndefinedToSchema, addMockFunctionsToSchema } from "graphql-tools";
import { graphqlSync, graphql } from "graphql";

describe("graphql-tools", () => {
	// 全部抽出しているけど全部試すのはめんどくさい…
	// クライアント側開発に使えそうなものだけ取り上げる

	it("makeExecutableSchema", async () => {
		// 型定義とResolverを与えて実行可能なschemaを作る
		// #@@range_begin(makeExecutableSchema)
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
		// #@@range_end(makeExecutableSchema)
		expect(result).toMatchSnapshot();
	});
	it("addCatchUndefinedToSchema", async () => {
		// resolverがundefinedを返したらエラーにしてくれる
		// ちゃんとnullを返しましょう
		const typeDefs = `
			type Query {
				foo: String
			}
		`;
		const resolvers = {
			Query: {
				foo: () => void 0,
			},
		};

		const schema = makeExecutableSchema({
			typeDefs,
			resolvers,
		});
		addCatchUndefinedToSchema(schema);

		const result = await graphql(schema, `{ foo }`);
		expect(result).toMatchSnapshot();
	});
	it.skip("addErrorLoggingToSchema", () => {
	});

	it.skip("mockServer", () => {
		// 単なる addMockFunctionsToSchema のラッパ
	});
	it("addMockFunctionsToSchema", async () => {
		// #@@range_begin(addMockFunctionsToSchema)
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
		// #@@range_end(addMockFunctionsToSchema)
		expect(result).toMatchSnapshot();
	});
	it.skip("MockList", () => {
	});

	it.skip("makeRemoteExecutableSchema", () => {
	});
	it.skip("introspectSchema", () => {
	});
	it.skip("mergeSchemas", () => {
	});
	it.skip("delegateToSchema", () => {
	});
	it.skip("defaultMergedResolver", () => {
	});
	it.skip("defaultCreateRemoteResolver", () => {
	});

	it.skip("transformSchema", () => {
	});
	it.skip("AddArgumentsAsVariables", () => {
	});
	it.skip("CheckResultAndHandleErrors", () => {
	});
	it.skip("ReplaceFieldWithFragment", () => {
	});
	it.skip("AddTypenameToAbstract", () => {
	});
	it.skip("FilterToSchema", () => {
	});
	it.skip("RenameTypes", () => {
	});
	it.skip("FilterTypes", () => {
	});
	it.skip("RenameTypes", () => {
	});
	it.skip("TransformRootFields", () => {
	});
	it.skip("RenameRootFields", () => {
	});
	it.skip("FilterRootFields", () => {
	});
	it.skip("ExpandAbstractTypes", () => {
	});
	it.skip("ExtractField", () => {
	});
	it.skip("WrapQuery", () => {
	});

	it.skip("SchemaDirectiveVisitor", () => {
	});
});
