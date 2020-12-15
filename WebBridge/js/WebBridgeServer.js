
class CalculatorService extends Service {
  constructor() {
    super("org.rdk.Calculator");
    this.registerMethod('add', 1, this.add.bind(this));
    this.registerMethod('sub', 1, this.sub.bind(this));
  }

  add(params) {
    console.log("add(" + JSON.stringify(params) + ")")
    return new Promise((resolve, reject) => {
      let sum = params.reduce((a, b) => a + b, 0);
      resolve(sum)
    });
  }

  sub(params) {
    console.log("sub(" + JSON.stringify(params) + ")")
    return new Promise((resolve, reject) => {
      let diff = params[0] - params[1]
      resolve(diff)
    });
  }
}

const service_manager = new ServiceManager()
service_manager.open({"host":"127.0.0.1", "port":10001})
  .then(() => {
    service_manager.registerService(new CalculatorService())
  }).catch(ex => {
    console.log(ex.error);
  });
