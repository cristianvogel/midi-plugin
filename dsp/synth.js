import invariant from 'invariant';
import {el} from '@elemaudio/core';


export default function synth(props, xl, xr) {
  invariant(typeof props === 'object', 'Unexpected props object');

  return [xl, xr];   // thru
}
