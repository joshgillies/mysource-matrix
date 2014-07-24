<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: weatherco_feed.inc,v 1.9 2012/08/30 00:58:16 ewang Exp $
*
*/





require_once SQ_PACKAGES_PATH.'/data/data_source_xml/data_source_xml.inc';

/**
* Weather Company Feed
*
* Purpose
*
*
* @author  Basil Shkara <bshkara@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage data
*/
class Weatherco_Feed extends Data_Source_Xml
{


	/**
	* Weather the asset is deprecated or not.
	* this will be used in asset_map.inc:_cloneAsset() to see if the asset type
	* can be created (cloned) on the system
	*/
	public $deprecated = TRUE;


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
			$year = date('y');
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
