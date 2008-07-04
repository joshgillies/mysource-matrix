	function init()	{
		if (GBrowserIsCompatible()) {
			map = new GMap2(document.getElementById("map"));
			//retrieveMarkers();
			var location = new GLatLng(centerLatitude, centerLongitude);
			map.setCenter(location, zoomLevel);
			map.setMapType(map_type);
			getCoveredArea(centerLatitude, centerLongitude, 3.0, "#000000", .1, 0.75, "#ffcc66",.2);
		    //addMarker(centerLatitude, centerLongitude);
	    }//end if

	}//end init()


	function addListeners(name)	{
		switch (name) {
			case 'latlngMouseLocation' :
				GEvent.addListener(map, 'mousemove', function(latlng) {
					var pixelLocation  = map.fromLatLngToDivPixel(latlng);
				 	GLog.write('Latitude Longtitude:' + latlng + ' at PIXEL LOCATION:' + pixelLocation);
				 }//end function
				)//end addListener
				break;

			case 'zoomChange' :
				GEvent.addListener(map, 'zoomend', function(oldLevel, newLevel) {
				      GLog.write('ZOOM CHANGED from '+ oldLevel + ' to ' + newLevel);
				     }//end function
				)//end addListener
				break;

			case 'addMarker' :
				GEvent.addListener(map, 'click',
					function(overlay, latlng) {
						if (icon_url != '' && icon_url != null) {
							var icon = new GIcon();
							icon.image = icon_url;
							icon.iconSize = new GSize(icon_width, icon_height);
							icon.iconAnchor  = new GPoint(14, 25);
							icon.infoWindowAnchor = new GPoint(14, 14);
							var marker  = new GMarker(latlng, icon);
						} else {
							var marker  = new GMarker(latlng);
						}//end else
						map.addOverlay(marker);
					}//end function
				);
				break;

		}//end switch
	}//end addListeners()


	function addInputFormListener(html)
	{
		GEvent.addListener(map, "click",
	      function(overlay, latlng) {
	        var inputForm  = document.createElement("form");
	        inputForm.setAttribute("action", "");
	        //inputForm.onsubmit  = function() { storeMarker(); return false;};

	        var lng  = latlng.lng();
	        var lat  = latlng.lat();
	        inputForm.innerHTML = html;

	       map.openInfoWindow(latlng, inputForm);
	      }//end function
	    );//end addListener
	}


	function addMarker(name, latitude, longitude, icon_image, description) {
		if (icon_image != null && icon_image != '') {
			var icon_url_final = icon_image;
		} else {
			var icon_url_final = icon_url;
		}//end if
		if (icon_url_final != '' && icon_url_final != null) {
			var icon = new GIcon();
			icon.image = icon_url_final;
			icon.iconSize = new GSize(icon_width, icon_height);
			icon.iconAnchor  = new GPoint(icon_width/2, 25);
			icon.infoWindowAnchor = new GPoint(icon_width/2, 14);
			var marker  = new GMarker(new GLatLng(latitude, longitude), icon);
		} else {
			var marker  = new GMarker(new GLatLng(latitude, longitude));
		}//end else

		GEvent.addListener(marker, 'click',
		  function () {
		    //map.showMapBlowup(new GLatLng(latitude, longitude), 5, G_HYBRID_MAP);
		    marker.openInfoWindowHtml(description);
		  }//end function
		)//end addListener

		map.addOverlay(marker);
		markerList[name] = marker;
	}//end function


	function storeMarker() {
		var lng  = document.getElementById("longitude").value;
		var lat  = document.getElementById("latitude").value;

		var getVars = ["?found=" , document.getElementById("found").value
		      ,  "&depth=" , document.getElementById("depth").value
		      ,  "&icon=" , document.getElementById("icon").value
		      ,  "&lng="  , lng
		      ,  "&lat="  , lat].join('');

		var request = GXmlHttp.create();

		//Open the request to storeMarker.php on the server
		request.open('GET', 'storeMarker.php' + getVars, true);
		request.onreadystatechange = function() {

		  if (request.readyState == 4) {
		      var xmlDoc  = request.responseXML;

		      // get the root node
		      var responseNode = xmlDoc.documentElement;

		      // get the type attribute
		      var type  = responseNode.getAttribute("type");

		      // get the content
		      var content = responseNode.firstChild.nodeValue;

		    if (type != 'success') {
		      alert(content);
		    } else {
		      //create a new marker
		      var latlng  = new GLatLng(parseFloat(lat), parseFloat(lng));
		      var iconImage  = responseNode.getAttribute("icon");
		      var marker  = createMarker(latlng, content, iconImage);
		      map.addOverlay(marker);
		      map.closeInfoWindow();
		    }//end else
		  }//end if
		}//end function
		request.send(null);
		return false;

	}//end storeMarker()


	function createMarker(latlng, html, iconImage) {
		if (iconImage!='') {
		  var icon = new GIcon();
		  icon.image = "http://delta.squiz.net/~hnguyen/Images/"+iconImage+".jpg";
		  icon.iconSize = new GSize(30, 30);
		  icon.iconAnchor  = new GPoint(14, 25);
		  icon.infoWindowAnchor = new GPoint(14, 14);
		  var marker  = new GMarker(latlng, icon);
		  //map.addOverlay(marker);

		} else {
		  var marker  = new GMarker(latlng);
		}//end else

		//var marker  = new GMarker(latlng);
		GEvent.addListener(marker, 'click', function() {
		  var markerHTML = html;
		  marker.openInfoWindowHtml(markerHTML);
		 }//end function
		)//end addListener
		return marker;
	}//end createMarker


	function retrieveMarkers() {
		var request  = GXmlHttp.create();

		request.open('GET', 'retrieveMarkers.php', true);
		request.onreadystatechange = function() {

		  if (request.readyState == 4) {
		    var xmlDoc = request.responseXML;
		     //console.info(xmlDoc);

		    var markers = xmlDoc.documentElement.getElementsByTagName("marker");
		   // console.info(markers);

		    for (var i = 0; i < markers.length; i++) {
		      var lng  = markers[i].getAttribute("lng");
		      var lat  = markers[i].getAttribute("lat");
		      if (lng && lat) {
		        var latlng  = new GLatLng(parseFloat(lat), parseFloat(lng));

		        var html  = ['<div><b>Found </b>',
		                markers[i].getAttribute("found"),
		                '</div><div><b>Depth</b> ',
		                markers[i].getAttribute("depth"),
		                '</div>'].join('');

		        var iconImage = markers[i].getAttribute("icon");
		        if (iconImage == null) iconImage = '';
		        var marker  = createMarker(latlng, html, iconImage);
		        map.addOverlay(marker);

		      }//end if

		    }//end for
		  }//end if

		}//end function
		request.send(null);

	}//end retrieveMarkers


	function setMapType(mapType)
	{
		map.setMapType(mapType);
	}//end setMapType


	/* This function from Kip - Pamela - Dave */
	function getCoveredArea(lat, lng, radius, strokeColor, strokeWidth, strokeOpacity, fillColor, fillOpacity)
	{
		var d2r = Math.PI/180;
		var r2d = 180/Math.PI;
		var Clat = radius * 0.014483;
		var Clng = Clat/Math.cos(lat*d2r);
		var Cpoints = [];
		for (var i=0; i < 33; i++) {
		  var theta = Math.PI * (i/16);
		  Cy = lat + (Clat * Math.sin(theta));
		  Cx = lng + (Clng * Math.cos(theta));
		  var P = new GPoint(Cx,Cy);
		  Cpoints.push(P);
		}
		//console.info(Cpoints);
		var polygon = new GPolygon(Cpoints, strokeColor, strokeWidth, strokeOpacity, fillColor, fillOpacity);
		map.addOverlay(polygon);
	}//end getCoveredArea


