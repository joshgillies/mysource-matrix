<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: db_connector.inc,v 1.2.8.1 2009/11/29 22:23:27 bpearson Exp $
*
*/

check_ssv_licence();
require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_PACKAGES_PATH.'/data/data_source_db/data_source_db.inc';

/**
* Db_Connector
*
* Purpose
* 	Used by DB data sources to store DB connection settings and construct the DB object
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.2.8.1 $
* @package MySource_Matrix_Packages
* @subpackage data
*/
class Db_Connector extends Asset
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
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	* @param int		$contextid	what context to return the name from
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE, $contextid=NULL)
	{
		// No context specified, using the current context
		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}//end if

		// Obtain the attribute value for Name from the specified Context
		$values = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('name', $this->type(), Array($this->id), $contextid);
		if (empty($values) === TRUE) {
			return parent::_getName($short_name, $contextid);
		} else {
			return $values[$this->id];
		}

	}//end _getName()


	/**
	* Get the DSN array for use by DB Data Source and others
	*
	* @return array
	* @access public
	*/
	function getDSN()
	{
		$asset = new Data_Source_DB();
		return $asset->constructDSNArray($this);

	}//end getDSN()


}//end class

?>
