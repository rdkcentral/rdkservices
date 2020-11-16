import Storage from '../src/localCookie.js'

let expect = chai.expect

describe('local Storage', () => {
	let ls = new Storage()
	it('should return an object', () => {
		expect(ls).to.be.a('object')
	})

	it('should have a setItem function', () => {
		expect(ls.setItem).to.be.a('function')
	})

	it('should have a removeItem function', () => {
		expect(ls.removeItem).to.be.a('function')
	})

	it('should have a clear function', () => {
		expect(ls.clear).to.be.a('function')
	})

	it('should accept a key/value "a" with value "b"', () => {
		let _r = ls.setItem('a','b')
		expect( _r ).to.be.a('undefined')
	})
	it('should return the same value "b" for key "a" ', () => {
		let _r = ls.getItem('a')
		expect( _r ).to.be.equal('b')
	})

	it('should store a number as a string', () => {
		ls.setItem('anumber', 1)
		expect( ls.getItem('anumber') ).to.be.a('string')
	})

	it('should removeItem key "a"', () => {
		let _r = ls.removeItem('a')
		expect( _r ).to.be.a('undefined')
	})

	it('should return a null value for key "a"', () => {
		let _r = ls.getItem('a')
		expect( _r ).to.be.a('null')
	})

	it('set key "a" and key "b"', () => {
		ls.setItem('a', 'a')
		ls.setItem('b', 'b')
	})

	it('call clear()', () => {
		expect( ls.clear() ).to.be.a('undefined')
	})

	it('should not have a value for key "a"', () => {
		expect( ls.getItem('a') ).to.be.a('null')
	})

	it('should not have a value for key "b"', () => {
		expect( ls.getItem('b') ).to.be.a('null')
	})
})

describe('Cookies ', () => {
	let cs = new Storage({ forceCookies : true })
	it('should return an object', () => {
		expect(cs).to.be.a('object')
	})

	it('should have a setItem function', () => {
		expect(cs.setItem).to.be.a('function')
	})

	it('should have a removeItem function', () => {
		expect(cs.removeItem).to.be.a('function')
	})

	it('should have a clear function', () => {
		expect(cs.clear).to.be.a('function')
	})

	it('should accept a key/value "a" with value "b"', () => {
		let _r = cs.setItem('a','b')
		expect( _r ).to.be.a('undefined')
	})
	it('should return the same value "b" for key "a" ', () => {
		let _r = cs.getItem('a')
		expect( _r ).to.be.equal('b')
	})

	it('should store a number as a string', () => {
		cs.setItem('anumber', 1)
		expect( cs.getItem('anumber') ).to.be.a('string')
	})

	it('should removeItem key "a"', () => {
		let _r = cs.removeItem('a')
		expect( _r ).to.be.a('undefined')
	})

	it('should return a null value for key "a"', () => {
		let _r = cs.getItem('a')
		expect( _r ).to.be.a('null')
	})

	it('set key "a" and key "b"', () => {
		cs.setItem('a', 'a')
		cs.setItem('b', 'b')
	})

	it('call clear()', () => {
		expect( cs.clear() ).to.be.a('undefined')
	})

	it('should not have a value for key "a"', () => {
		expect( cs.getItem('a') ).to.be.a('null')
	})

	it('should not have a value for key "b"', () => {
		expect( cs.getItem('b') ).to.be.a('null')
	})
})

mocha.run()