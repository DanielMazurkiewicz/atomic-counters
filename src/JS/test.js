const { openCounters, prepare, current, next, add, release, closeCounters } = require('./index');

console.log(require('./index.js'))

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

console.log(currentValue, incrementCounter, addValueToCounter)