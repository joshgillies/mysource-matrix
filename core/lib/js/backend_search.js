/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: backend_search.js,v 1.2 2006/11/20 04:23:45 lwright Exp $
*
* backend_search.js
*
* Utility function(s) and variables that drive the backend quick search feature.
* This should only be included into the "main" frame.
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix
*/


/**
* Current position in the search results
* @var int
*/
current = 0;


/**
* Results per page (by default - will be changed when a search is run)
* @var int
*/
results_per_page = 5;


/**
* Placeholder for search results, so we can tab through them
* @var array
*/
keyword_search_results = [];


/**
* Jump to a specific page in backend search results
*
* @param int	start	the position to start from (zero-based)
*
* @return void
*/
function jumpToSearchResults(start)
{
	document.getElementById("sq-search-results-page-start").innerHTML = start + 1;
	document.getElementById("sq-search-results-page-end").innerHTML = Math.min(start + results_per_page, keyword_search_results.length);
	for (i = 1; i <= results_per_page; i++) {
		result_num = start + i - 1;
		if (result_num >= keyword_search_results.length) {
			document.getElementById("sq-search-results-entry-" + i).style.display = 'none';
			document.getElementById("sq-search-results-expand-" + i).style.display = 'none';
		} else {
			document.getElementById("sq-search-results-entry-" + i).innerHTML = keyword_search_results[result_num];
			document.getElementById("sq-search-results-expand-link-" + i).innerHTML = '+';
			document.getElementById("sq-search-results-entry-" + i).style.display = 'block';
			document.getElementById("sq-search-results-expand-" + i).style.display = 'block';

			// For some reason the class name needs to be reinforced, otherwise
			// the text indent override provided by this class does not work
			// properly
			document.getElementById("sq-search-results-detail-" + i).className = 'sq-search-results-detail';
			document.getElementById("sq-search-results-detail-" + i).style.display = 'none';

		}
	}
}//end jumpToSearchResults()
