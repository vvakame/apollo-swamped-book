import "./index.css";

import React from "react";
import { render } from "react-dom";
import { ApolloClient } from "apollo-client";
import { ApolloProvider } from "react-apollo";
import { InMemoryCache } from "apollo-cache-inmemory";
import gql from "graphql-tag";

import { link } from "./graphql/link";
import { App } from "./App";

const typeDefs = gql`
  extend type Query {
    currentJsonData: JSONContainer!
  }
  type JSONContainer {
    id: ID
    data: JSON!
  }
  type Mutation {
    setCurrentJsonData(id: ID!): JSONContainer!
  }
`;

const cache = new InMemoryCache();
cache.writeData({
  data: {
    __typename: "Query",
    currentJsonData: {
      __typename: "JSONContainer",
      id: null,
      data: null
    }
  }
});
const client = new ApolloClient({
  cache,
  link,
  typeDefs,
  resolvers: {
    Mutation: {
      async setCurrentJsonData(_, { id }, { cache, client }) {
        console.log("setCurrentJsonData", id, cache, client);
        const resp = await client.query({
          query: gql`
            query GetSingleJSON {
              jsonData
            }
          `
        });
        console.log("resp", resp);
        cache.writeData({
          data: {
            __typename: "Query",
            currentJsonData: {
              __typename: "JSONContainer",
              id: "JSONContainer:1",
              data: resp.data.jsonData
            }
          }
        });
        return resp.data.jsonData;
      }
    }
  }
});

render(
  <ApolloProvider client={client}>
    <App />
  </ApolloProvider>,
  document.getElementById("root")
);
