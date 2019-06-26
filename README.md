
# Installation

```
yarn add atomic-counters
```
or
```
npm install atomic-counters
```

# Usage

```javascript
import { openCounters, prepare, current, next, release, closeCounters } from 'atomic-counters';

const growBlockSize = 1024;
const initializationTimeoutInMs = 500;
const timeoutInMs = 500;
const countersDb = openCounters('file_name', growBlockSize, initializationTimeoutInMs, timeoutInMs);
const counter = prepare(countersDb, 'counter');
const valueWrittenInDb = current(counter);
const incrementCounter = next(counter);
release(counter); // free counter resources at the end of counter use
closeCounters(countersDb);
```

# Where to use

Counters are persistent and works across multiple processes/forks, so best to use them especially in such conditions.
If you work with databases and need safe serial incrementing numbers before inserting record then this lib is a right choice for you.

# What next?

If you've found bug, need some extra functionality, or so, drop an info in issues in this repo or post a fix/update...