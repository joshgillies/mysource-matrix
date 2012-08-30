<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: saml2_acs_management.inc,v 1.2 2012/08/30 00:59:03 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Saml2_ACS_Management
*
* @see Asset_Management
*
* @author  Edison Wang <ewang@squiz.cm.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage saml
*/

class Saml2_ACS_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		parent::__construct($pm);
		$this->vars = Array(
						'name'			=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
		    
			);

	}//end constructor


}//end class
?>
