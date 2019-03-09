import React from "react";
import { Query, Mutation } from "react-apollo";
import gql from "graphql-tag";

export const query = gql`
  query TopQuery {
    currentJsonData @client {
      id
      data
    }
  }
`;

export const setCurrentJsonData = gql`
  mutation SetCurrentJsonData($id: ID!) {
    setCurrentJsonData(id: $id) @client {
      id
      data
    }
  }
`;

export const App = () => {
  return (
    <Query query={query}>
      {({ loading, error, data: queryData }) => {
        return (
          <main>
            {loading ? (
              <p>Loading…</p>
            ) : (
              <Mutation
                mutation={setCurrentJsonData}
                refetchQueries={() => {
                  return [{ query: query }];
                }}
              >
                {(execMut, { loading, error, ..._rest }) => {
                  const onClick = () => {
                    execMut({ variables: { id: "JSONContainer:1" } });
                  };
                  return (
                    <div>
                      <button onClick={onClick}>EXECUTE MUTATION</button>
                      <br />
                      user edit the JSON data & send mutation to server.
                      <ul>{JSON.stringify(queryData)}</ul>
                    </div>
                  );
                }}
              </Mutation>
            )}
          </main>
        );
      }}
    </Query>
  );
};
