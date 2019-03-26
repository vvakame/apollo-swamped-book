import gql from "graphql-tag";
import { InMemoryCache } from "apollo-cache-inmemory";
import { WriteCacheFragment1 } from "./graphql/WriteCacheFragment1";
import { WriteCacheFragment2 } from "./graphql/WriteCacheFragment2";
import { ReadCacheFragment } from "./graphql/ReadCacheFragment";

const cache = new InMemoryCache({
	dataIdFromObject: obj => obj.id,
});

// apollo client:codegen が完走する→schemaに対して不正なQueryではないことがわかる
// apolloが生成した型を型パラメータとして利用するとdataの書式もチェックできる
cache.writeFragment<WriteCacheFragment1>({
	id: "Cat:yukari",
	fragment: gql`
		fragment WriteCacheFragment1 on Cat { id kind }
	`,
	data: {
		__typename: "Cat",
		id: "Cat:yukari",
		kind: "norway jan forest cat & ragdoll",
	},
});
cache.writeFragment<WriteCacheFragment2>({
	id: "Cat:yukari",
	fragment: gql`
		fragment WriteCacheFragment2 on Cat { kind name }
	`,
	data: {
		__typename: "Cat",
		kind: null,
		name: "yukari",
	},
});

const result = cache.readFragment<ReadCacheFragment>({
	id: "Cat:yukari",
	fragment: gql`
		fragment ReadCacheFragment on Cat {
			id
			kind
			name
		}
	`,
});
// 結果は統合される
// 書き込みフィールドが被った場合は後勝ち
// {"id":"Cat:yukari","kind":null,"name":"yukari","__typename":"Cat"}
console.log(JSON.stringify(result));
