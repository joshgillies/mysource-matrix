<?php

/**
* Cron_Manager
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Cron_Manager extends Asset
{
	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Cron_Manager($assetid=0)
	{
		$this->Asset($assetid);
	}


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

		$this->_tmp[__CLASS__.'_in_create'] = true;

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT count(*) FROM sq_asset WHERE type_code = '.$db->quote($this->type());
		$exists = $db->getOne($sql);
		if (DB::isError($exists)) trigger_error($exists->getMessage().'<br/>'.$exists->getUserInfo(), E_USER_ERROR);

		if ($exists) {
			trigger_error('Unable to create another '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name').', you are only allowed one', E_USER_WARNING);
			return false;
		}

		$this->setAttrValue('name', 'Trash');

		$ret_val = parent::create($link);
		unset($this->_tmp[__CLASS__.'_in_create']);
		return $ret_val;

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
		return $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name');
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
					 SQ_LINK_TYPE_2 => Array(),
					 SQ_LINK_TYPE_3 => Array('cron_job' => Array('card' => 'M', 'exclusive' => true)),
					 SQ_LINK_NOTICE => Array()
					);

	}//end _getAllowedLinks()


	/**
	* Creates a duplicate of this asset. It DOES NOT duplicate the links that this asset has either to it or from it
	* Returns a reference to the new object or if an error occured FALSE
	*
	* @param bool	$dupe_directory	whether or not to duplicate thia assets data directory
	*
	* @return mixed object Asset or NULL
	* @access public
	* @see Asset::duplicate()
	*/
	function &duplicate($dupe_directory=true)
	{
		trigger_error('Unable to duplicate '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name').', you are only allowed one', E_USER_WARNING);
		$null = NULL; // 'cause we need to return by reference :)
		return $null;
	}//end duplicate()


	/**
	* Morph's this asset to either one of it's parent type or one of it's children types
	* Only succeeds if $type_code asset's is allowed to link to all children that this asset has
	*
	* @param string	$new_type_code	the new type_code to attempt to cast this asset to
	*
	* @return bool
	* @access public
	* @see Asset::morph()
	*/
	function morph($new_type_code)
	{
		trigger_error('Unable to morph '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'), E_USER_WARNING);
		return false;
	}//end morph()


	/**
	* Moves a link from one place in this assets links order to another position
	*
	* @param int	$linkid		the link id of the link to move about
	* @param string	$sort_order	the position in the links list that this link should take,
	*							if less than zero places at end of list
	*
	* @see Asset::moveLinkPos()
	*
	* @return boolean
	* @access public
	*/
	function moveLinkPos($linkid, $sort_order=-1)
	{
		trigger_error('You cannot move assets\'s around in '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'), E_USER_WARNING);
		return false;
	}//end moveLinkPos()


	/**
	* Sets a permission for the passed user or user_group
	*
	* @param int		$userid		the id of the user or user_group to add the permission for
	* @param string		$permission	the permission code you are adding
	* @param string		$access		whether this permission grants access or not ('1' or '0')
	*
	* @return boolean
	* @access public
	*/
	function setPermission($userid, $permission, $access)
	{
		trigger_error('You cannot set permissions on the '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'), E_USER_WARNING);
		return false;
	}// end setPermission()

	/**
	* Sets a permission for the passed user or user_group
	*
	* @param object User	$user		a reference to the user that this job will run as
	* @param string			$type_code	the specific type of cron_job that this job needs to be
	*
	* @return mixed object Cron_Job or null on error
	* @access public
	*/
	function &addJob(&$user, $type_code)
	{
		$null = NULL; // needed because we are returning by reference

		$job_types = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('cron_job');
		if (!in_array($type_code, $job_types)) {
			trigger_error('"'.$type_code.'" is not an asset type that inherites from "cron_job"', E_USER_WARNING);
			return $null;
		}

		$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_3, 'exclusive' => '1');
		$job = new $type_code();
		if ($job->create($link)) { 
			if ($job->setRunningUser($user)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				return $job;
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		return $null;

	}// end addJob()

}//end class
?>
