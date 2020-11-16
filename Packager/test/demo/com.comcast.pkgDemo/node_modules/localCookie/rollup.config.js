import babel from 'rollup-plugin-babel'
import resolve from 'rollup-plugin-node-resolve'
import commonjs from 'rollup-plugin-commonjs'
import uglify from 'rollup-plugin-uglify-es';
import peerDepsExternal from 'rollup-plugin-peer-deps-external'

export default [
  {
    input: './src/localCookie.js',
    output: {
      file: './dist/localCookie.js',
      format: 'iife',
      name: 'Storage',
    },
    plugins: [resolve({ browser: true }), commonjs(), babel(), uglify()],
  },
  {
    input: './src/localCookie.js',
    output: {
      file: './module/localCookie.js',
      format: 'esm',
      name: 'Storage',
    },
    plugins: [peerDepsExternal(), babel(), uglify()],
  },
]
