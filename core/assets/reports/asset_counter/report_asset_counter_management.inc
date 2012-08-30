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
* $Id: report_asset_counter_management.inc,v 1.7 2012/08/30 01:09:09 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Report_Asset_Counter_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Report_Asset_Counter_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars['types']		= Array(
										'added'		=> '1.1',
										'type'		=> 'serialise',
										'default'	=> Array(),
									  );

		$this->vars['sort_by']		= Array(
										'added'			=> '0.0.1',
										'type'			=> 'selection',
										'parameters'	=> Array(
															'options'		=> Array(
																				'count'	=> 'Count',
																				'name'	=> 'Name',
																				'type'	=> 'Code',
																			   ),
															'multiple'		=> FALSE,
															'allow_empty'	=> FALSE,
														   ),
										'default'		=> 'type',
									  );

		$this->vars['reverse_sort']	= Array(
										'added'			=> '0.1',
										'type'			=> 'boolean',
										'default'		=> FALSE,
										'parameters'	=> Array(
															'allow_empty'	=> FALSE,
														   ),
									  );

	}//end constructor


}//end class

?>