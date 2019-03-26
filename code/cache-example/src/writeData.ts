import { InMemoryCache } from "apollo-cache-inmemory";

const cache = new InMemoryCache({
	// Relay Global Object Identification に従い、IDだけで一意になるようにする
	// InMemoryCacheのデフォルトだと `${obj.__typename}:${obj.id}` などが使われる
	dataIdFromObject: obj => obj.id,
});

// 次のような __typename がないぞ！というエラーが2回出る
// Missing field __typename in {
//   "date": "2019/03/26",
//   "event": "爪が伸びたのでかいぬしをプスってした"
// }
cache.writeData({
	data: {
		notExists: true, // schemaに存在しないデータもお咎めなし
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

// 内部的に作成されたキャッシュは次の通り…
// ROOT_QUERY
// Cat:yukari
// Cat:yukari.data.0 ←余計 JSONの中まで見てる
// Cat:yukari.data.1 ←余計 JSONの中まで見てる
Object.keys(cache.extract()).forEach(cacheKey => console.log(cacheKey));
