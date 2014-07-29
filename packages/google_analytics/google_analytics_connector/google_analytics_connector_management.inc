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
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Google Analytics Connector Management
*
* @see Asset_Management
*
* @author  Squiz Pty Ltd <products@squiz.net>
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Google_Analytics_Connector_Management extends Asset_Management
{


	/**
	* Constructor
	*
	* @param Package_Manager	$pm	The package manager relevant to this particular asset
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);
		
		// Period.
		$this->vars['period'] = Array(
                                    'added'			=> '0.1',
                                    'type'			=> 'selection',
                                    'default'		=> 'P1M',
                                    'description'	=> 'The period to generate. Key is to be an ISO8601 period.',
                                    'parameters'	=> Array(
                                                        'multiple'		=> FALSE,
                                                        'allow_empty'	=> FALSE,
                                                        'options'		=> Array(
                                                                            'P1M'	=> 'Previous month',
                                                                            'P3M'	=> 'Previous 3 months',
                                                                            'P6M'	=> 'Previous 6 months',
                                                                           ),
                                                       ),
                                );

	}//end constructor


}//end class
?>
