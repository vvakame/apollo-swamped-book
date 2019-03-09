import { graphql, print } from "graphql";
import { ApolloLink, Observable } from "apollo-link";
import { schema } from "./schema";

export const link = new ApolloLink(operation => {
  return new Observable(observer => {
    const { query, operationName, variables } = operation;

    (async () => {
      try {
        await delay(800);
        const result = await graphql(schema, print(query), null, null, variables, operationName);
        observer.next(result);
        observer.complete();
      } catch (e) {
        observer.error(e);
      }
    })();
  });
});

function delay(ms: number) {
  return new Promise(resolve => {
    setTimeout(() => {
      resolve();
    }, ms);
  });
}
