= Apollo Linkで魔術する

apollo-clientはなかなか素敵な仕組みを持っています。
Apollo Link@<fn>{apollo-link}です。
これを使うと様々な魔術が可能になるので、これについて取り上げます。

//footnote[apollo-link][@<href>{https://www.apollographql.com/docs/link/}]

== Apollo Linkの概要

Apollo Linkはapollo-clientがOperationをなんらかの結果に変換するための仕組みです。
イメージとしては、ミドルウェアを事前に登録しておくと、clientへのリクエストがミドルウェアによって解決されていく感じです。
実際に、ネットワーク経由でサーバのGraphQLエンドポイントを叩く処理もこのApollo Linkの仕組みで処理されています。

皆さんがほぼ確実に使っているのは、apollo-link-http@<fn>{npm-apollo-link-http}でしょう（@<list>{apollo-link-http-example}）。

//footnote[npm-apollo-link-http][@<href>{https://www.npmjs.com/package/apollo-link-http}]

//list[apollo-link-http-example][apollo-link-httpの例]{
const client = new ApolloClient({
  link: new HttpLink({ uri: "/api/graphql" }),
  cache: new InMemoryCache(),
});
//}

クライアントのへのリクエストはapollo-link-httpのHttpLinkでハンドルされ、サーバ上のGraphQLエンドポイントへのリクエストに変換されます。
その結果がHttpLinkから送り返されてきた結果、GraphQLのクエリが処理されたように見えるわけです。

ApolloLinkはざっくり@<list>{ApolloLink-definition}のような定義になっています。
コンストラクタに実装を与えるか、ApolloLinkをextendsしてrequestメソッドをオーバーライドして実装します。

//list[ApolloLink-definition][ApolloLinkの型定義（抜粋）]{
export interface Operation {
  // 一部省略 & 変更
  query: DocumentNode;
  variables: { [name: string]: any; };
  operationName: string;
}
// FetchResult の定義は割愛
export declare type NextLink = (operation: Operation) => Observable<FetchResult>;
export declare type RequestHandler =
  (operation: Operation, forward?: NextLink) => Observable<FetchResult> | null;
export declare class ApolloLink {
  constructor(request?: RequestHandler);
  request(
    operation: Operation,
    forward?: NextLink,
  ): Observable<FetchResult> | null;
}
//}

== Apollo Linkでサーバ側実装なしにコードを組む

まずは肩慣らしに、デモ作成の時に便利なネットワークアクセス無しでクエリを投げて結果を得られるようなものを作ってみます。
発想としては、@<chapref>{utilities}で紹介した関数を組み合わせ、schema.jsonからschemaを作成し、それにmockのresolverを仕込み、それに対してクエリを投げます。
実装は@<list>{link-example/src/index.ts}のようになりました。

//list[link-example/src/index.ts][虚空からレスポンスをひねり出す]{
#@mapfile(../code/link-example/src/index.ts)
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
#@end
//}

ApolloLinkでObservableを作成し、返す。
リクエストが来たら、それをschemaとgraphql関数などを駆使して結果を作成し、返す。
このclientはqueryが投げられたとしても、完全にローカルで処理するのでネットワークアクセスが発生しません。
つまり、サーバ無しに開発を進めることができるわけです。
便利ですね。

この例を見てわかるのは、リクエストはOperationとして各linkに到達し、linkは自分がそれを処理できるのであれば何らかの方法で結果を作成して返せればよい、ということです。
GraphQLではトランスポートレイヤーの規定などは使用上何も存在しないため、様々なプロトコルにリクエストを変換して乗せることができるのです。

== Subscriptionを何かしらのPushとQueryに変換する

本題です。
筆者が愛するGoogle AppEngineはWebSocketなどのリアルタイムなPushを行う手段がありません。
WebSocketがなかったら折角GraphQLにはSubscriptionという面白い仕組みがありキャッシュによる画面の自動更新とかもあるというのにそれが生かされないだろ…！？と思っていました。
しかし、Apollo Linkの仕組みを使えば、Subscriptionのリクエストを何らかのPushサービス+Queryによるデータの取得に変換可能できそうです。

具体的に、Firestoreなどのクライアント側からsubscribe可能なマネージドサービスetcを準備します。
サーバ側からは、そのPushサービスを介してクライアント側にIDと変更の種類（Created/Updated/Deleted）を送ります。
クライアント側では、IDから頑張ってQueryを逆算して、リクエストを投げ、その結果をSubscriptionの結果であるように誤魔化します。

こうすることにより、WebSocketが利用できない環境でもGraphQL Subscriptionが使える…。
かもしれない？というアイディアです。
なかなか頭が悪そうなので実現させるためにはかなりの無茶と馬力が必要そうではあります。

ざっくりした実装を@<href>{https://github.com/vvakame/til/pull/17}に置いておいたので興味があったら見てみてください。

//comment{
 * https://github.com/apollographql/graphql-subscriptions
//}
