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
* $Id: web_folder.inc,v 1.7 2012/08/30 01:09:16 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/site/site.inc';

/**
* Web_Folder
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.7 $
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
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

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
	public function create(Array &$link)
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
	protected function _createAdditional(Array &$link)
	{
		$root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);
		$url_suffix = '_'.substr($this->type(), 11);

		$urls = Array();
		$urls[] = Array(
					'url'				=> $root_urls[0].'/'.$url_suffix,
					'http'				=> 1,
					'https'				=> 0,
					'base_contextid'	=> 0,
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
	protected function _getName($short_name=FALSE)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name');

	}//end _getName()


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* @return boolean
	* @access public
	*/
	public function canDelete()
	{
		return FALSE;

	}//end canDelete()


	/**
	* Return FALSE always because we don't want to be able to clone a cache manager
	*
	* @return boolean
	* @access public
	*/
	public function canClone()
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
	public function _getAllowedLinks()
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
