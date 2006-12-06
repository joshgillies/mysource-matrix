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
* $Id: metadata_field_thesaurus_management.inc,v 1.2 2006/12/06 01:07:36 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Metadata_Field_Thesaurus_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Thesaurus_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Metadata_Field_Thesaurus_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'thesaurus_assetid'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'assetid',
												'default'		=> '',
												'description'	=> 'Thesaurus asset which will supply the data to this metadata field.',
											   ),
					  );

	}//end constructor


}//end class

?>