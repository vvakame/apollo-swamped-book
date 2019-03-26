// from https://blog.apollographql.com/three-ways-to-represent-your-graphql-schema-a41f4175100d
//      https://blog.apollographql.com/modularizing-your-graphql-schema-code-d7f71d5ed5f2

const fs = require("fs");
const glob = require("glob");
const { graphqlSync, getIntrospectionQuery } = require("graphql");
const { makeExecutableSchema } = require("graphql-tools");

const typeDefs = glob
    .sync("./*.graphql")
    .map(file => fs.readFileSync(file, { encoding: "utf8" }));

const graphqlSchemaObject = makeExecutableSchema({
    typeDefs: typeDefs,
    resolverValidationOptions: {
        requireResolversForResolveType: false,
    },
});

{
    const introspectionQuery = getIntrospectionQuery();
    const result = graphqlSync(graphqlSchemaObject, introspectionQuery).data;
    fs.writeFileSync("schema.json", JSON.stringify(result, null, 2));
}
{
    // from https://www.apollographql.com/docs/react/advanced/fragments.html#fragment-matcher
    const result = graphqlSync(graphqlSchemaObject, `
        {
            __schema {
                types {
                    kind
                    name
                    possibleTypes {
                        name
                    }
                }
            }
        }
    `);
    const filteredData = result.data.__schema.types.filter(
        type => type.possibleTypes !== null,
    );
    result.data.__schema.types = filteredData;
    fs.writeFileSync('./src/apolloTypes/fragmentTypes.ts', `export const introspectionQueryResultData = ${JSON.stringify(result.data, null, 2)};`);
}
