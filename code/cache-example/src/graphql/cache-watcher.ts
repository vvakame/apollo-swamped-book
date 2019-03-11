import { InMemoryCache, InMemoryCacheConfig } from 'apollo-cache-inmemory';
import { Cache } from 'apollo-cache';

export class CacheWatcher extends InMemoryCache {
  constructor(config: InMemoryCacheConfig) {
    super(config);
  }

  write(write: Cache.WriteOptions) {
    let before = Object.keys((this as any).data.data).length;
    super.write(write);
    let after = Object.keys((this as any).data.data).length;
    if (before === after) {
      console.warn(`cache.write is ignored`, write);
    }
  }
}

