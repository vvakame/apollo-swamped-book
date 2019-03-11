import gql from "graphql-tag";

const viewerQuery = gql`
  query ViewerQuery {
    viewer {
      # もちろんこれ以外にも色々ある
      id
      bio
      here:location
    }
  }
`;
