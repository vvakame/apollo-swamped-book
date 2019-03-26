/* tslint:disable */
/* eslint-disable */
// This file was automatically generated and should not be edited.

// ====================================================
// GraphQL query operation: ReadCacheQuery
// ====================================================

export interface ReadCacheQuery_cat {
  __typename: "Cat";
  id: string;
  name: string;
  kind: string | null;
}

export interface ReadCacheQuery {
  cat: ReadCacheQuery_cat | null;
}
