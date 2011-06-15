<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: trigger_action_make_soap_call_management.inc,v 1.1.2.2 2011/06/15 23:40:15 akarelia Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Trigger_Action_Make_SOAP_Call_Management
*
* @see Asset_Management
*
* @author  Ash Karelia <akarelia@squiz.com.au>
* @version $Revision: 1.1.2.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Make_SOAP_Call_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		$this->Asset_Management($pm);

	}//end constructor


}//end class

?>
