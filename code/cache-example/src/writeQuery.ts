import gql from "graphql-tag";
import { InMemoryCache } from "apollo-cache-inmemory";
import { WriteCacheQuery } from "./graphql/WriteCacheQuery";

const cache = new InMemoryCache({
	dataIdFromObject: obj => obj.id,
});

// apollo client:codegen が完走する→schemaに対して不正なQueryではないことがわかる
// apolloが生成した型を型パラメータとして利用するとdataの書式もチェックできる
cache.writeQuery<WriteCacheQuery>({
	query: gql`
		query WriteCacheQuery {
			cats {
				id
				kind
				name
				data
			}
		}
	`,
	data: {
		// notExists: true, // ←型でエラーになる & 与えてもキャッシュに書く際に無視される
		cats: [
			{
				__typename: "Cat",
				id: "Cat:yukari",
				kind: "norway jan forest cat & ragdoll",
				name: "yukari",
				data: [
					{
						date: "2019/03/26",
						event: "爪が伸びたのでかいぬしをプスってした",
					},
					{
						date: "2019/03/27",
						event: "あくびしてるところの写真を撮られた",
					},
				],
			},
		],
	},
});

// 内部的に作成されたキャッシュは次の通り キレイ！
// Cat:yukari
// ROOT_QUERY
Object.keys(cache.extract()).forEach(cacheKey => console.log(cacheKey));
