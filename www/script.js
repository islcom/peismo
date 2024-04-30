

//import { parseDataRecords } from 'seisplotjs-miniseed';

async function fetchAnd() {

	dataRecords1 = await Promise.all(filenamesFromPhp.map(async fn => {
		const miniseedData = await fetch('uploaded/' + fn);
		return seisplotjs.miniseed.parseDataRecords(await miniseedData.arrayBuffer());
	}));

	dataRecords = dataRecords1.flat();

	const plotEnd = seisplotjs.moment.utc().endOf('hour').add(1, 'millisecond');
	if (plotEnd.hour() % 2 === 1) {plotEnd.add(1, 'hour');}
	const timeWindow = new seisplotjs.util.StartEndDuration(null, plotEnd, seisplotjs.moment.duration(1, 'day'));
	//new seisplotjs.fdsndatacenters.DataCentersQuery().findFdsnDataSelect("IRISDMC")

	let seismogram = seisplotjs.miniseed.seismogramPerChannel(dataRecords)[0];

	let heliConfig = new seisplotjs.helicorder.HelicorderConfig(timeWindow);
	heliConfig.numLines = 48
	heliConfig.height = 800;
	heliConfig.title = `Helicorder for ${seismogram.codes()}`;

	let seisData = seisplotjs.seismogram.SeismogramDisplayData.fromSeismogram(seismogram);
	seisData.addMarkers([ { markertype: 'predicted', name: "now", time: seisplotjs.moment.utc() } ]);
	let helicorder = new seisplotjs.helicorder.Helicorder("div#helicorder",
											heliConfig,
											seisData);
	helicorder.draw();	
}

fetchAnd();