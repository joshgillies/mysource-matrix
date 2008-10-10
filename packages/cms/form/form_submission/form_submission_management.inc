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
* $Id: form_submission_management.inc,v 1.21 2008/10/10 03:52:51 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Submission_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.21 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Submission_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Form_Submission_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'id'				=> Array(
												'added'		=> '0.0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'attributes'		=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'array with keys "answers", "summary", "extra_data"',
											   ),
						'complete'			=> Array(
												'added'			=> '0.2',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'description'	=> 'TRUE if submission complete, FALSE if incomplete and not yet submitted.',
											   ),
						'captcha_status'	=> Array(
												'added'			=> '0.2',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'description'	=> 'TRUE if a CAPTCHA has been correctly submitted for this form so far.',
											   ),
						'xml'				=> Array(
												'added'		=> '0.2',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'current_page'		=> Array(
												'added'		=> '0.3',
												'type'		=> 'int',
												'default'	=> 0,
												'description'	=> 'The page the user last saw on this submission, updated with the current page when a user jumps back and forth through the form',
												'is_admin'	=> FALSE,
											   ),
						'latest_page'		=> Array(
												'added'		=> '0.3',
												'type'		=> 'int',
												'default'	=> 0,
												'description'	=> 'The last page the user saw - ie. the page furthest along the line that has been seen, irrespective of whether a user backtracked to a previous page',
												'is_admin'	=> FALSE,
											   ),
					  );

	}//end constructor


}//end class
?>
