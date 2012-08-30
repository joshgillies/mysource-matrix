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
* $Id: ipb_poll.inc,v 1.9 2012/08/30 00:58:55 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Ipb_Poll
*
* Purpose
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage ipb
*/
class Ipb_Poll extends Asset
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
	function Ipb_Poll($ipbid=0, $data=Array())
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
		if ($this->_suffix != '0') {
			$this->name = '('.$data['poll_state'].') '.$data['poll_question'];
		} else {
			$this->name = $data['poll_question'];
		}

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

		if ($this->_suffix != '0') {
			$this->vars['pid']['type'] = 'int';
			$this->vars['pid']['value'] = $data['pid'];

			$this->vars['tid']['type'] = 'int';
			$this->vars['tid']['value'] = $data['tid'];

			$this->vars['start_date']['type'] = 'int';
			$this->vars['start_date']['value'] = $data['start_date'];

			$this->vars['choices']['type'] = 'text';
			$this->vars['choices']['value'] = $data['choices'];

			$this->vars['starter_id']['type'] = 'int';
			$this->vars['starter_id']['value'] = $data['starter_id'];

			$this->vars['votes']['type'] = 'int';
			$this->vars['votes']['value'] = $data['votes'];

			$this->vars['forum_id']['type'] = 'int';
			$this->vars['forum_id']['value'] = $data['forum_id'];

			$this->vars['poll_question']['type'] = 'text';
			$this->vars['poll_question']['value'] = $data['poll_question'];

			$this->vars['poll_state']['type'] = 'text';
			$this->vars['poll_state']['value'] = $data['poll_state'];
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
	* Returns name of the user
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE)
	{
		return 'IPB Polls';

	}//end _getName()


}//end class

?>
