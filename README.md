
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
import { openCounters, prepare, current, next, add, release, closeCounters } from 'atomic-counters';

const growBlockSize = 1024;
const initializationTimeoutInMs = 500;
const timeoutInMs = 500;
const countersDb = openCounters('file_name', growBlockSize, initializationTimeoutInMs, timeoutInMs);
const counter = prepare(countersDb, 'counter');
const currentValue = current(counter); // returns undefined if counter was not used yet
const incrementCounter = next(counter); // starts counting from 0
const addValueToCounter = add(counter, 9); 
release(counter); // free counter resources at the end of counter use
closeCounters(countersDb);
```

# Where to use

Counters are persistent and works across multiple processes/forks, so best to use them especially in such conditions.
If you work with databases and need safe serial incrementing numbers before inserting record then this lib is a right choice for you.

# How it works

It uses memory mapped files and atomic operations to make sure that it will work across multiple processes and make sure it will be fast. Counters are 64 bit

# What next?

If you've found bug, need some extra functionality, or so, drop an info in issues in this repo or post a fix/update...