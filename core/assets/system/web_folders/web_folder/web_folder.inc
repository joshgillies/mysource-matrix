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
* $Id: web_folder.inc,v 1.3 2006/01/26 22:34:07 lwright Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/site/site.inc';

/**
* Web_Folder
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Web_Folder extends Site
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Web_Folder($assetid=0)
	{
		$this->Site($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* Returns either the linkid or FALSE on failure
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return mixed int|boolean
	* @access public
	* @see Asset::create()
	*/
	function create(&$link)
	{
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';
		if (!system_asset_fns_create_pre_check($this)) {
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($linkid = parent::create($link)) {
			if (!system_asset_fns_create_cleanup($this)) {
				$linkid = FALSE;
			}
		}

		if ($linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $linkid;

	}//end create()


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Web Folders add a default URL when created
	*
	* @param array	&$link	information used to create the initial link<br/>
	* <pre>
	* Array ('asset'         => [ref major asset to create link under],
	*        'link_type'     => SQ_LINK_?,
	*        'value'         => [link value],
	*        'sort_order'    => [link sort order],
	*        'is_dependant'  => [0|1],
	*        'is_exclusive'  => [0|1]
	*        )
	* </pre>
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		$root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);
		$url_suffix = '_'.substr($this->type(), 11);

		$urls = Array();
		$urls[] = Array(
					'url'	=> $root_urls[0].'/'.$url_suffix,
					'http'	=> 1,
					'https'	=> 0,
				  );

		if (!$this->saveWebUrls($urls)) return FALSE;
		return $this->updateLookups();

	}//end _createAdditional()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	*/
	function _getName($short_name=FALSE)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name');

	}//end _getName()


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* @return boolean
	* @access public
	*/
	function canDelete()
	{
		return FALSE;

	}//end canDelete()


	/**
	* Return FALSE always because we don't want to be able to clone a cache manager
	*
	* @return boolean
	* @access public
	*/
	function canClone()
	{
		return FALSE;

	}//end canClone()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{

		return Array(
				SQ_LINK_TYPE_1	=> Array(
									'folder'	=> Array(
													'card'		=> 'M',
													'exclusive'	=> FALSE,
												   ),
								   ),
				SQ_LINK_TYPE_2	=> Array(
									'folder'	=> Array(
													'card'		=> 'M',
													'exclusive'	=> FALSE,
												   ),
								   ),
				SQ_LINK_NOTICE	=> Array(
									'design'	=> Array(
													'card'		=> 'M',
													'exclusive'	=> FALSE,
												   ),
								   ),
			   );

	}//end _getAllowedLinks()


}//end class

?>
