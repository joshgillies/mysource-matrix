<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: ipb_forum.inc,v 1.9 2012/08/30 00:58:55 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Ipb_Forum
*
* Purpose
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage ipb
*/
class Ipb_Forum extends Asset
{

	/**
	* The ID of the IPB bridge we are from
	*
	* @var int
	*/
	var $_ipbid = 0;

	/**
	* The first character of this asset.
	*
	* @var string
	*/
	var $_prefix = '';

	/**
	* The last character of this asset.
	*
	* @var string
	*/
	var $_suffix = '';


	/**
	* Constructor
	*
	* @param int	$ldapid	the ID of the IPB bridge that links to the IPB for this user
	* @param array	$data	the IPB entry for this user (contains all user data)
	*/
	function Ipb_Forum($ipbid=0, $data=Array())
	{
		$this->_ser_attrs = TRUE;
		$this->_loadVars();

		if (!$ipbid || empty($data)) {
			return;
		}

		$ipb_bridge =& $GLOBALS['SQ_SYSTEM']->am->getAsset($ipbid, 'ipb_bridge');
		if (is_null($ipb_bridge)) return;

		$this->_ipbid = $ipbid;

		// set general object properties
		$tmp = explode('_', $data['id']);
		$this->_prefix = $tmp[0];
		$this->_suffix = $tmp[1];
		$this->id = $ipb_bridge->id.':'.$data['id'];
		$this->name = $data['name'];
		$this->name = $data['name'];

		$this->short_name = $this->name;
		$this->status = $ipb_bridge->status;
		$this->version = '0.1';
		if (!empty($ipb_bridge->created)) {
			$this->created = date('Y-m-d H:m:i', $ipb_bridge->created);
		}
		if (!empty($ipb_bridge->updated)) {
			$this->updated = date('Y-m-d H:m:i', $ipb_bridge->updated);
		}
		if (!empty($ipb_bridge->published)) {
			$this->published = date('Y-m-d H:m:i', $ipb_bridge->published);
		}
		if (!empty($ipb_bridge->status_changed)) {
			$this->status_changed = date('Y-m-d H:m:i', $ipb_bridge->status_changed);
		}

		// set the data paths
		$this->_loadDataPaths();

		// Ipb_Post doesn't have 'name' and 'description'
		if ($this->_prefix != 'p') {
			$this->vars['name']['type'] = 'text';
			$this->vars['name']['value'] = $data['name'];

			$this->vars['description']['type'] = 'text';
			if (!empty($data['description'])) {
				$this->vars['description']['value'] = $data['description'];
			}
		}

		// category
		if ($this->_prefix == 'f' && $this->_suffix == '0') {
			// number of root forums
			$ipb_bridge =& $GLOBALS['SQ_SYSTEM']->am->getAsset($ipbid);
			$ipb =& $ipb_bridge->getIpbConn();
			$root_forums = $ipb->getListOfRootForums();

			$this->vars['root_forums']['type'] = 'int';
			$this->vars['root_forums']['value'] = count($root_forums);
		}

		// nomal forum
		if ($this->_prefix == 'f' && $this->_suffix != '0') {
			$this->vars['topics']['type'] = 'int';
			$this->vars['topics']['value'] = $data['topics'];
		}

	}//end constructor


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		return Array();

	}//end _getAllowedLinks()


	/**
	* Returns name of the forum
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE)
	{
		return 'IPB All Forums';

	}//end _getName()


}//end class

?>
