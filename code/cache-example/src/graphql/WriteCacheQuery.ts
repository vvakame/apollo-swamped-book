/* tslint:disable */
/* eslint-disable */
// This file was automatically generated and should not be edited.

// ====================================================
// GraphQL query operation: WriteCacheQuery
// ====================================================

export interface WriteCacheQuery_cats {
  __typename: "Cat";
  id: string;
  kind: string | null;
  name: string;
  data: any | null;
}

export interface WriteCacheQuery {
  cats: WriteCacheQuery_cats[] | null;
}
