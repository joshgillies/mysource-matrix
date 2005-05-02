<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: package_manager_lexicon.inc,v 1.1.1.1.2.1 2005/05/02 00:35:49 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_INCLUDE_PATH.'/package_manager.inc';

/**
* Package_Manager_Lexicon
*
* Purpose
*    Manages the install and upgrade of the Lexicon system assets, 
*    uses info gleaned from the package.xml and asset.xml files
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.1.1.1.2.1 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage lexicon
*/
class Package_Manager_Lexicon extends Package_Manager
{

	/**
	* Constructor
	*
	*/
	function Package_Manager_Lexicon()
	{
		$this->_full_path = dirname(__FILE__);
		$this->Package_Manager();

	}//end constructor

}//end class

?>