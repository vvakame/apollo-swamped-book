import gql from "graphql-tag";

const viewerQuery = gql`
  query ViewerQuery {
    viewer {
      # 必要な分だけ書けばいい！
      id
      bio
      here:location
    }
  }
`;
