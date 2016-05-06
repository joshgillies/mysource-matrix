<?php
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
* $Id: physical_folder.inc,v 1.3 2012/08/30 00:58:36 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Physical Folder
*
* Purpose
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage file
*/
class Physical_Folder extends Asset
{

	/**
	* the asset ID of the image that the question belongs to
	* @var int
	*/
	var $_bridgeid = 0;

	/**
	* the shadow ID of the variety - unique for each parent asset
	* @var int
	*/
	var $_shadowid = 0;


	/**
	* Constructor
	*
	* @param string	$assetid    the asset id to be loaded
	* @param Array	$data		the shadow asset data that will construct this variety
	*
	*/
	function __construct($assetid=0, $data=Array())
	{
		parent::__construct();

		if (!$assetid) return;
		list($this->_bridgeid, $this->_shadowid) = explode(':', $assetid);

		$bridge = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_bridgeid);
		if (is_null($bridge)) return;

		// set the data paths
		$this->data_path        = $bridge->data_path.'/physical_folder';
		$this->data_path_suffix = $bridge->data_path_suffix.'/physical_folder';
		$this->data_path_public = $bridge->data_path_public.'/physical_folder';

		// if there is no shadowid then we haven't been created yet, so don't do anymore
		if (!$this->_shadowid) return;

		$this->id = $assetid;

		foreach ($data as $attr_name => $attr_value) {
			if (!isset($this->vars[$attr_name])) continue;
			$this->vars[$attr_name]['value'] = ($this->vars[$attr_name]['type'] == 'serialise') ? unserialize($attr_value) : $attr_value;
		}

		$location = $bridge->getLocationPath();
		$location = rtrim('/', $location);
		if(!empty($location) && strpos($location, '/') !== 0) $location = SQ_SYSTEM_ROOT.'/'.$location;
		
		$full_path = $location.'/'.$this->_shadowid;
		$name = basename($full_path);

		// set general object properties, now we have the name of the thing
		$this->name = $name;
		$this->short_name = $name;
		$this->status = $bridge->status;
		$this->version = '0.1';

	}//end constructor

}//end class
?>
