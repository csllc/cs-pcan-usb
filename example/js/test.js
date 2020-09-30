const events = require('events');
const Can = require('../..');


let can = new Can({
  canRate: 500000,
  filters: [
    //{ext: true, id: '00000081 000000F8'},
    //{ext: true, code: 0x00000081, mask: 0x000000F8},
    //{ext: true, code: 0x000000E0, mask: 0x0000001F}
    //{ext: true, id: '10EF0000 10EFFFFF'},
    //{ext: true, id: '10ED0000 10EE0000'},
    //{ext: true, code: 0x10EF0000, mask: 0xFFFF0000},
    {ext: true, id: '01EE0000 01EFFFFF'}
  ]
});


can.list()
  .then((ports) => {
    console.log(ports);
    return can.open(ports[0].path)
      .catch((err) => {
	console.error(err);
	process.exit(1);
      });
  })
  .catch((err) => {
    console.error(err);
    can.close();
    process.exit(1);
  });

can.on('data', function(msg) {
  console.log(JSON.stringify(msg, (key, value) => {
    if ((typeof value === 'number') && (key == "id")) {
      return value.toString(16).toUpperCase().padStart(8, '0');
    } else if (typeof value === 'number') {
      return value.toString(16).toUpperCase().padStart(2, '0');
    } else {
      return value;
    }
  }));
});

