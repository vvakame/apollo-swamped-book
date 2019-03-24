import { graphql, print, parse, buildClientSchema, } from "graphql";
import { addMockFunctionsToSchema } from "graphql-tools";

import { ApolloClient } from "apollo-client";
import { InMemoryCache } from "apollo-cache-inmemory";

import { ApolloLink, Observable } from "apollo-link";

// Introspection Queryの結果のJSONを読み込む
import schemaJSON from "../schema.json";

// schema.jsonからschemaをでっち上げる
const schema = buildClientSchema(schemaJSON as any);
addMockFunctionsToSchema({ schema });

// クエリをmockで処理するLinkを組む
const client = new ApolloClient({
	link: new ApolloLink(operation => {
		return new Observable(observer => {
			// 要求されたリクエストを分解
			const { query, operationName, variables } = operation;
			// schemaに対してクエリを実行
			graphql(schema, print(query), null, null, variables, operationName)
				.then(result => {
					observer.next(result);
					observer.complete();
				})
				.catch(e => {
					observer.error(e);
				});
		});
	}),
	cache: new InMemoryCache(),
});

// クエリ投げる
client
	.query({
		query: parse(`query Mock { viewer { id } }`),
	})
	.then(result => {
		// mockデータが返ってくる！
		console.log(JSON.stringify(result, null, 2));
	});
