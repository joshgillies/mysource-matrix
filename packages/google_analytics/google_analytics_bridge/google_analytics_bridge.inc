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
* $Id: data_record.inc,v 1.9 2013/01/29 06:42:16 akarelia Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/oauth2/oauth2_token/oauth2_token.inc';
require_once SQ_CORE_PACKAGE_PATH.'/interfaces/bridge/bridge.inc';

/**
* Google Analytics Bridge
*
* Purpose
*
*
* @author  Squiz Pty Ltd <products@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage google_analytics
*/
class Google_Analytics_Bridge extends OAuth2_Token implements Bridge
{


    /**
     * The authentication endpoint.
     *
     * The "access_type=offline" query param is to ensure we get a refresh
     * token. Google doesn't provide this by default.
     *
     * @const string
     */
    const AUTH_ENDPOINT = 'https://accounts.google.com/o/oauth2/auth?access_type=offline';
    
    /**
     * The token endpoint.
     *
     * @const string
     */
    const TOKEN_ENDPOINT = 'https://accounts.google.com/o/oauth2/token';
    
    /**
     * Space-separated list of scopes desired by this bridge.
     *
     * The plain-English permissions shown to users by Google when
     * requesting permission are, in order:
     * - View your Google Analytics data
     * - Edit Google Analytics management entities
     *
     * User Management and Provisioning API access is not requested.
     *
     * @const string
     */
    const DEFAULT_SCOPE = 'https://www.googleapis.com/auth/analytics.readonly https://www.googleapis.com/auth/analytics.edit';
    
    /**
     * Default fetch prefix for Analytics API data.
     *
     * @const string
     */
    const FETCH_PREFIX = 'https://www.googleapis.com/analytics/v3';
    
    
	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

	}//end constructor


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Pages add a default web path when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _createAdditional(Array &$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		// Certain items of the token storage are not meant to be user-updateable.
		$this->setAttrValue('authorization_endpoint', self::AUTH_ENDPOINT);
		$this->setAttrValue('token_endpoint', self::TOKEN_ENDPOINT);
		$this->setAttrValue('scope', self::DEFAULT_SCOPE);
		$this->setAttrValue('token_storage', 'global');

		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
		$this->saveAttributes();
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
		
		return TRUE;

	}//end _createAdditional()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE, $contextid=NULL)
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{
		$links = parent::_getAllowedLinks();
		$links[SQ_LINK_TYPE_2]['google_analytics_property'] = Array('card' => 'M', 'exclusive' => FALSE);

		return $links;

	}//end _getAllowedLinks()
	
	
//--      FRONTEND PAINTING        --//

    
	/**    
	 * Outputs the requested resource(s).
	 *  
	 * @return void    
	 * @access public    
	 */    
	public function printFrontend()
	{
		// don't let browser/proxy cache this asset
		header('Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0');
		header('Pragma: no-cache');
		header('Expires: '.gmdate('D, d M Y H:i:s', time()-3600).' GMT');
		
		// start authentication (if required or forced)
		// Unlike the OAuth2 Token, "force=true" is required, not "force" with
		// any value.
		// Eventually force won't exist, revoking of token to occur in the
		// edit interface because we have a fixed place to do this.
		$force = FALSE;
		if (array_get_index($_GET, 'force') === 'true') {
		    $force = TRUE;
		}
		
		$token = $this->getAccessToken($force);
		return parent::printFrontend();
		
	}//end printFrontend()
	
	
	/**
	 * Paint the body once authenticated.
	 */
	public function printBody()
	{
	    $token = $this->getAccessToken(FALSE);
	    echo json_encode($token);
	    echo json_encode($this->getProperties());
	    
	}//end printBody()
	
	
//--      ANALYTICS REQUESTING        --//


    public function getProperties()
    {
        $url = self::FETCH_PREFIX.'/management/accountSummaries';
        $response = $this->fetch($url);
        return $response;
        
    }//end getProperties()
    
    
    public function revokeAccessToken()
    {
        
        
    }//end revokeAccessToken()


//--      BRIDGE FUNCTIONS        --//

	
	/**
	* Returns a reference to the asset represented by the passed assetid
	*
	* Returns NULL on error
	*
	* @param int		$shadowid		the shadow part asset id to be loaded (i.e part after the ':')
	* @param string		$type_code		if this exists then this object is used to
	*									load the asset, if not then the DB is queried
	*									to find out the asset type
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed because
	*									you can't use the '@' operator when returning by reference
	*
	* @return mixed object|NULL
	* @access public
	* @see Asset_Manager::&getAsset()
	*/
	public function getAsset($shadowid, $type_code='', $mute_errors=FALSE)
	{
	    $result = NULL;
	    
	    return $result;
	    
	}//end getAsset()


	/**
	* Return all links that this asset has to other assets
	*
	* @param int		$assetid			id of the the $side_of_link asset
	* @param int		$link_types			integer that can be the product of bitwise operations
	*										on the SQ_LINK_* constants
	* @param mixed		$type_code			the type of asset that is linked
	*										(eg 'Page', 'File', etc)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	* @param string		$side_of_link		Which side of the link this (the current) asset
	*										is on ('major' or 'minor')
	* @param string		$sort_by			a field in the asset table you would like the results
	*										sorted by (eg. name, short_name etc)
	*
	* @return array
	* @access public
	* @see Asset_Manager::getLinks()
	*/
	public function getLinks($assetid, $link_types, $type_code='', $strict_type_code=TRUE, $side_of_link='major', $sort_by=NULL)
	{
	    $result = Array();
	    
	    return $result;
	    
	}//end getLinks()


	/**
	* Get all asset ids that are above the passed assetid in the various trees in which it exists
	*
	* @param int		$assetid			the id of the asset to get its parents for
	* @param mixed		$type_code			the type of asset that is linked (eg 'User', 'User_Group', etc)
	*										if an array returns link if matches any of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	*
	* @return array
	* @access public
	* @see Asset_Manager::getParents()
	*/
	public function getParents($assetid, $type_code='', $strict_type_code=TRUE)
	{
	    $result = Array();
	    
	    return $result;
	    
	}//end getParents()


	/**
	* Get all asset ids that are below the passed assetid in the various trees in which it exists
	*
	* Returns an array of assetids and their type code
	*
	* @param int		$assetid			the id of the asset to get its children for
	* @param mixed		$type_code			the type of asset that is linked
	*										(eg 'Page', 'File', etc)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	* @param boolean	$dependant			if TRUE, results will be filtered to assets that are dependants.
	*										If FALSE, results will all not be dependants.
	*										If NULL, results will not be filtered
	* @param string		$sort_by			a field in the asset table you would like the results
	*										sorted by (eg. name, short_name etc)
	*
	* @return array
	* @access public
	* @see Asset_Manager::getChildren()
	*/
	public function getChildren($assetid, $type_code='', $strict_type_code=TRUE, $dependant=NULL, $sort_by=NULL)
	{
	    $result = Array();
	    
	    return $result;
	    
	}//end getChildren()


	/**
	* Returns an array of assetid's + info in the order that they propogate out from THIS BRIDGE'S url
	*
	* @param string	$assetid	the id of the last asset in the lineage
	* @param string	$protocol	the protocol to match -> NULL means it is ignored
	* @param string	$url		the url to check for -> NULL defaults it to current url
	*
	* @return array
	* @access public
	*/
	public function getLineageFromURL($assetid, $protocol, $url)
	{
	    $result = Array();
	    
	    return $result;
	    
	}//end getLineageFromURL()


	/**
	* Return the number of links in the system that involve the specified asset
	*
	* Can be restricted by either link type afnd/or asset type code
	*
	* @param int		$assetid			id of the the $side_of_link asset
	* @param string		$side_of_link		Which side of the link this (the current) asset
	*										is on ('major' or 'minor')
	* @param int		$link_types			integer that can be the product of bitwise operations
	*										on the SQ_LINK_* constants
	* @param mixed		$type_code			the type of asset that is linked (eg 'Page', 'File', etc) (can be a string or an array of strings)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	* @param int		$ignore_linkid		ignore the link represented by this link id when returning the count
	*
	* @return int
	* @access public
	*/
	public function countLinks($assetid, $side_of_link='major', $link_types=0, $type_code='', $strict_type_code=TRUE, $ignore_linkid=0)
	{
	    $result = 0;
	    
	    return $result;
	    
	}//end countLinks()


	/**
	* Remove a link by id
	*
	* @param int		$linkid	the link id of the link to remove
	* @param boolean	$moving	TRUE if this delete is part of a move operation
	*
	* @return boolean
	* @access public
	*/
	public function deleteAssetLink($linkid, $moving=FALSE)
	{
	    $ok = FALSE;
	    
	    return $ok;
	    
	}//end deleteAssetLink()


	/**
	* Get asset info for use by asset map
	*
	* Should return an array containing the following elements:<ul>
	* 	<li>assetid:    ID of the asset.</li>
	* 	<li>name:       Full name of the asset.</li>
	*	<li>short_name: Short name of the asset.</li>
	*	<li>version:    Asset version number.<li>
	*	<li>status:     Current status of the asset.</li>
	*	<li>type_code:  The type code of the shadow asset.</li>
	*   <li>num_kids:   Number of children, if known.</li>
	*	<li>accessible: Whether the asset should be accessible.</li>
	*	<li>url:        URL for this lineage of the asset, if known.</li>
	*	<li>web_path:   Web path for this lineage of the asset, if known.</li>
	*
	* @param string	$assetid	Full Asset id to get information for
	*
	* @return array
	* @access public
	*/
	public function getAssetMapAssetInfo($assetid)
	{
	    $result = Array();
	    
	    return $result;
	    
	}//end getAssetMapAssetInfo()


}//end class

?>
