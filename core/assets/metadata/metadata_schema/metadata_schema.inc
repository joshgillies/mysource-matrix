<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: metadata_schema.inc,v 1.4 2003/09/30 00:29:40 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Metadata_Schema
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Schema extends Asset
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Metadata_Schema($assetid=0)
	{
		$this->Asset($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* @param array()	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{
		$name = trim($this->attr('name'));

		if ($name == '') {
			trigger_error('Unable to create a '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name').' with a blank name', E_USER_WARNING);
			return false;
		}

		return parent::create($link);

	}//end create()


	/**
	* Returns name of the asset
	*
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @see Asset::_getName()
	* @access public
	*/
	function _getName($short_name=false)
	{
		return $this->attr('name');

	}//end _getName();


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(SQ_LINK_TYPE_1 => Array(),
					 SQ_LINK_TYPE_2 => Array('metadata_section' => Array('card' => 'M', 'exclusive' => false)),
					 SQ_LINK_TYPE_3 => Array(),
					 SQ_LINK_NOTICE => Array()
					);

	}//end _getAllowedLinks()


	/**
	* Creates a link between this asset and the passed asset, of a certain type
	* and returns the linkid of the created link
	*
	* @return int
	* @access public
	* @see Asset::creatLink();
	*/
	function createLink(&$minor, $link_type, $value='', $sort_order=-1, $dependant='0', $exclusive='0')
	{
		// if this is a metadata section then we need to make it a dependant link
		$regenerate = false;
		if (is_a($minor, 'metadata_section')) {
			$regenerate = true;
			$dependant = '1';
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!parent::createLink($minor, $link_type, $value, $sort_order, $dependant, $exclusive)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		if ($regenerate) {
			$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array('schemaid' => $this->id);
			$code_name = $hh->queueHipo('hipo_job_regenerate_metadata', $vars);
			if (empty($code_name)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}// end createLink()


	/**
	* Remove a link where this asset is the major player
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return boolean
	* @access public
	* @see Asset_Manager::deleteLink()
	*/
	function deleteLink($linkid)
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid, $this->id);
		$minor = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid']);
		$regenerate = false;
		if (!is_null($minor) && is_a($minor, 'metadata_section')) {
			$regenerate = true;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!parent::deleteLink($linkid)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		if ($regenerate) {
			$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array('schemaid' => $this->id);
			$code_name = $hh->queueHipo('hipo_job_regenerate_metadata', $vars);
			if (empty($code_name)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end deleteLink()


}//end class

?>