import { GraphQLSchema, GraphQLObjectType, GraphQLList } from "graphql";
import GraphQLJSON from "graphql-type-json";

const jsonData = [
  // { id: 1, name: "John Smith" },
  // { id: 2, name: "John Smith", data: { a: "a1" } },
  { id: 3, name: "John Smith", data: [{ a: "a1" }] },
  { id: 4, name: "Sara Smith", data: [{ a: "a2" }] },
  { id: 5, name: "Budd Deey", data: [{ a: "a3" }] }
];

const QueryType = new GraphQLObjectType({
  name: "Query",
  fields: {
    jsonData: {
      type: GraphQLJSON,
      resolve: (...args) => {
        console.log("jsonDataResolver", args);
        return jsonData[0];
      }
    },
    jsonDataList: {
      type: new GraphQLList(GraphQLJSON),
      resolve: () => jsonData
    }
  }
});

export const schema = new GraphQLSchema({ query: QueryType });
