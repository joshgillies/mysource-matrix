<?php
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
* $Id: remap_manager.inc,v 1.3 2004/02/18 18:19:16 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


/**
* Remap_Manager
*
* Manages the remapping of old URLs to new URLs
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.3 $ - 0.1
* @package MySource_Matrix_Packages
* @subpackage __core__
*/

class Remap_Manager extends Asset
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Remap_Manager($assetid=0)
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
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';
		if (!system_asset_fns_create_pre_check($this)) return false;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($linkid = parent::create($link)) {
			if (!system_asset_fns_create_cleanup($this))  {
				$linkid = false;
			}
		}

		if ($linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		return $linkid;

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
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* @return boolean
	* @access public
	*/
	function canDelete()
	{
		return false;

	}//end canDelete()


	/**
	* Add a URL remap to the system
	*
	* @param string	$old_url	the broken URL
	* @param string	$new_url	the URL to redirect to
	* @param int	$expires	timestamp for when remap expires (NULL for no expiry)
	*
	* @return int 0 on failed insert, 1 on success, -1 if URL already exists
	* @access public
	*/
	function addRemapURL($old_url, $new_url, $expires=null)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT COUNT(*)
				FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_lookup_remap';
		$where = 'url = '.$db->quote($old_url);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		$count = $db->getOne($sql.$where);
		if (DB::isError($count)) trigger_error($count->getMessage().'<br/>'.$count->getUserInfo(), E_USER_ERROR);

		if ((int)$count > 0) {
			trigger_error('Failed adding URL remap, URL "'.$old_url.'" is already remapping to another URL', E_USER_WARNING);
			return -1;
		}

		// check if the new URL is already remapping to something else, and delete it
		$where_cond = 'url = '.$db->quote($new_url);
		if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_lookup_remap', $where_cond)) return false;

		$values = Array('url'       => $old_url,
						'remap_url' => $new_url,
						'expires'   => (is_null($expires)) ? $expires : ts_iso8601($expires),
						);
		return (int)$GLOBALS['SQ_SYSTEM']->rollbackInsert('asset_lookup_remap', $values);

	}//end addRemapURL()


	/**
	* Remap all the URLs of an asset to a new URL
	*
	* @param string	$assetid	the ID of the asset we are adding remaps for
	* @param string	$new_url	the URL to redirect to
	* @param int	$expires	timestamp for when remap expires (NULL for no expiry)
	*
	* @return boolean
	* @access public
	*/
	function remapAllURLs($assetid, $new_url, $expires=null)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT url, http, https
				FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_lookup';
		$where = 'assetid = '.$db->quote($assetid);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		$urls = $db->getAll($sql.$where);
		if (DB::isError($urls)) trigger_error($urls->getMessage().'<br/>'.$urls->getUserInfo(), E_USER_ERROR);

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		foreach ($urls as $url_data) {
			if ($url_data['http']) {
				$retval = @$this->addRemapURL('http://'.$url_data['url'], $new_url, $expires);
				if ($retval === 0) {
					trigger_error('Failed remapping all URLs for Asset #'.$assetid.' to "'.$new_url.'"', E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
			}
			if ($url_data['https']) {
				$retval = @$this->addRemapURL('https://'.$url_data['url'], $new_url, $expires);
				if ($retval === 0) {
					trigger_error('Failed remapping all URLs for Asset #'.$assetid.' to "'.$new_url.'"', E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end remapAllURLs()


}//end class

?>