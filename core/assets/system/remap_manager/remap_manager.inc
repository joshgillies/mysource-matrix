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
* $Id: remap_manager.inc,v 1.8 2004/05/13 06:31:31 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Remap_Manager
*
* Manages the remapping of old URLs to new URLs
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.8 $ - 0.1
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
	* returns FALSE always because we don't want to be able to clone a remap manager
	*
	* @return boolean
	* @access public
	*/
	function canClone()
	{
		return false;
	
	}//end canClone()

	
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


	/**
	* Attempts to redirect a url to its remapped url
	* If the remap manager is unsuccessful in doing so, it will return false
	*
	* @param string		$protocol	the protocol that the url is using
	* @param string		$url		the url to remap from
	*
	* @return boolean
	* @access public
	*/
	function loadRemapFromURL($protocol=null, $url=null)
	{
		if (is_null($protocol)) $protocol = current_protocol();
		if (is_null($url)) $url = strip_url(current_url(false, true));

		$url = $protocol.'://'.$url;
		$db  =& $GLOBALS['SQ_SYSTEM']->db;

		$sql   = 'SELECT remap_url
				  FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_lookup_remap';
		$where = 'url = '.$db->quote($url).'
				  AND (expires IS NULL OR expires > NOW())';
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		$remap_url = $db->getOne($sql.$where);
		
		if (DB::isError($remap_url)) {
			trigger_error($remap_url->getMessage().'<br/>'.$remap_url->getUserInfo(), E_USER_ERROR);
		}

		// if we have a remap url, display it
		if (!empty($remap_url)) {
			?>
			<html>
				<head>
					<title>Redirecting you to <?php echo $remap_url; ?></title>
					<script language="javascript" type="text/javascript" src="<?php echo sq_web_path('lib');?>/js/general.js"></script>
				</head>
				<body onLoad="Javascript: sq_redirect('<?php echo $remap_url; ?>');"></body>
			</html>
			<?php

			return true;
		}

		return false;

	}//end loadRemapFromURL


	/**
	* Returns the asset that has been chosen to 
	* delegate 404 Not found requests
	*
	* @return &object asset or null
	* @access public
	*/
	function &getPageNotFoundAsset()
	{
		$null = null;
		if (!$this->attr('404_asset')) return null;
		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->attr('404_asset'));
		
		return $asset;

	}//end getPageNotFoundAsset()


	/**
	* Paints the page not found asset
	* 
	* If a design can be found for the asset, it will
	* be painted as well, otherwise justs the contents of
	* the asset are painted.
	*
	* @return boolean
	* @access public
	*/
	function paintPageNotFoundRemap()
	{
		$asset = &$this->getPageNotFoundAsset();
		if (is_null($asset)) return false;
		
		// send a 404 header
		header("HTTP/1.0 404 Not Found");
		
		$url = strip_url($this->getURL(), true);
		$asset_url = strip_url($asset->getURL(null, true), true);

		global $ASSET_LINEAGE;
		$ASSET_LINEAGE = $GLOBALS['SQ_SYSTEM']->am->getLineageFromUrl(null, $asset_url);

		$design = &$this->getDesign($asset, $url);
		if (!is_null($design)) {
			$design->paint($asset);
			return true;
		} 
		// get the first design for this asset
		$design = &$this->getDesign($asset);
		if (!is_null($design)) {
			$design->paint($asset);
			return true;
		}
		// give up, and just paint the asset
		// without a design
		$asset->printFrontend();
	
		return true;
	
	}//end paintPageNotFoundRemap()
	

	/**
	* Returns the design that this asset currently has applied to it.
	*
	* Pass in null as $url to return the first design asset associated
	* with the asset's id
	*
	* @param object &Asset $asset 	the asset to get the design for
	* @param $url 					the url of the asset
	*
	* @return &object Asset
	* @access public
	*/
	function &getDesign(&$asset, $url=null)
	{
		$null = null;
		if (is_null($asset)) return $null;
		
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT ld.designid, a.type_code
				FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_lookup l
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_lookup_design ld ON l.url = ld.url
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset a ON ld.designid = a.assetid
				';
		$where ='l.assetid  = '.$db->quote($asset->id);
		if (!is_null($url)) $where .= ' AND ld.url  LIKE '.$db->quote($url.'%');
		$where .= ' AND ld.name    = '.$db->quote('system_design::frontend');
		
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ld');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
		$sql .= $where.' LIMIT 1';

		$result = $db->getRow($sql);
		
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
			return $null;
		}
		
		if ($result) {
			return $GLOBALS['SQ_SYSTEM']->am->getAsset($result['designid'], $result['type_code']);
		}
		return $null;
	
	}//end getDesign()
	

}//end class

?>