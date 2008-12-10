<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: weatherco_feed.inc,v 1.5 2008/12/10 05:49:41 bshkara Exp $
*
*/

check_ssv_licence();



require_once SQ_PACKAGES_PATH.'/data/data_source_xml/data_source_xml.inc';

/**
* Weather Company Feed
*
* Purpose
*
*
* @author  Basil Shkara <bshkara@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage data
*/
class Weatherco_Feed extends Data_Source_Xml
{


    /**
    * Constructor
    *
    * @param int    $assetid    the asset id to be loaded
    *
    */
    function __construct($assetid=0)
    {
        parent::__construct($assetid);

	}//end constructor


	/**
	* Sends GET request and calls the parsing process
	*
	* @return mixed array|boolean
	* @access public
	*/
	function parseXML()
	{
		// calculate Weather Co. rolling authentication key
		$url = html_entity_decode($this->attr('url'));
		$userid = $this->attr('userid');
		$password = $this->attr('password');
		if (!empty($userid) && !empty($password)) {
			$day = date('j');
			$month = date('n');
			$year = str_replace('0', '', date('y'));
			$authkey = md5((($day * 2) + ($month * 300) + ($year * 170000)).$password);
			$weatherfeed_url = $url.'&u='.$userid.'&k='.$authkey;
			$content = $this->openFeed($weatherfeed_url);
		} else {
			return Array();
		}

		return $this->_createParser($content);

	}//end parseXML()


}//end class
?>
