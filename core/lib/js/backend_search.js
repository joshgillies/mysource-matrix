/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: backend_search.js,v 1.4 2012/08/30 01:09:21 ewang Exp $
*
* backend_search.js
*
* Utility function(s) and variables that drive the backend quick search feature.
* This should only be included into the "main" frame.
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix
*/


/**
* Current position in the search results
* @var int
*/

var MatrixBackendSearch = {
	currentPage: 0,
	totalResults: 0,
	resultsPerPage: 5,
	
	next: function() {
		var lastPage = Math.ceil(this.totalResults / this.resultsPerPage) - 1;
		if (this.currentPage < lastPage) {
			this.jump(this.currentPage + 1);
		}
	},
	
	back: function() {
		if (this.currentPage > 0) {
			this.jump(this.currentPage - 1);
		}
	},
	
	first: function() {
		this.jump(0);
	},
	
	last: function() {
		this.jump(Math.ceil(this.totalResults / this.resultsPerPage) - 1);
	},
	
	jump: function(page) {
		var oldEls = document.querySelectorAll(".sq-search-results-page-" + this.currentPage);
		for (var i = 0; i < oldEls.length; i++) {
			oldEls[i].style.display = 'none';
		}
		
		this.currentPage = page;
		
		var newEls = document.querySelectorAll(".sq-search-results-page-" + this.currentPage);
		for (var i = 0; i < newEls.length; i++) {
			newEls[i].style.display = 'block';
		}
		
		document.getElementById("sq-search-results-page-start").innerHTML = ((page * this.resultsPerPage) + 1);
		document.getElementById("sq-search-results-page-end").innerHTML = Math.min(((page+1) * this.resultsPerPage), this.totalResults);
	
	}
};
