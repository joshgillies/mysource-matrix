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
* $Id: page_poll_management.inc,v 1.3 2005/04/29 05:40:31 gsherwood Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Poll_Management
*
* @see Asset_Management
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Robert Howard <rhoward@squiz.net>
* @author  David Heppell <dheppell@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Poll_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Page_Poll_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
					// currently active question
					'active_question' => Array(
								'added' => '0.1',
								'type'  => 'assetid',
								'default'	=> ''
								),

					// mode for voting
					'Vote_Behaviour' => Array(
								'added' => '0.1',
								'type'  => 'selection',
								'default'	=> 'sessions',
								'parameters' => Array(
											'options' => Array(
														'sessions' => 'Sessions',
														'login' => 'Login',
														'none' => 'None',
											),
											'multiple' => false,
											'allow_empty' => false
								)
					),

					// graph box
					'graph_box_width'	=> Array(
								'added'	=> '0.1',
								'type'	=> 'text',
								'default'	=> '400'
					),

					'graph_box_height'	=> Array(
								'added'	=> '0.1',
								'type'	=> 'text',
								'default'	=> '400'
					),

					'graph_box_colour_bar'	=> Array(
								'added' => '0.1',
								'type'  => 'colour',
								'default'	=> '#0000cc'
					),

					// screen WYSIWYGs
					'question_format' => Array(
								'added'     => '0.1',
								'type'      => 'wysiwyg',
								'default'   =>
									'<div style="border: 1px solid #000066; width: 420px; padding: 5px">
										<div style="align: center;">
										   <strong>%question%</strong> <em>(%date%)</em>
										</div>
										<div style="list-style: none">
										   %option_list%
										</div>
										<div style="text-align: center;">
											<div style="text-align: left; display: inline;">%view_results%</div>
										<div style="text-align: center; display: inline;">%vote_button%</div>
											<div style="text-align: right; display: inline;">%view_past_questions%</div>
										</div>
									</div>'
					),

					'results_format' => Array(
								'added'     => '0.1',
								'type'      => 'wysiwyg',
								'default'   =>
												'<div style="border: 1px solid #000066; width: 420px; padding: 5px">
													<div style="align: center;">
													   <strong>%question%</strong> <em>(%date%)</em>
													</div>
													<div style="list-style: none">
													%graph%<br />
													%key%
													</div>
													<div style="text-align: center;">
													<div style="text-align: left; display: inline;">&nbsp;</div>
													<div style="text-align: center; display: inline;">&nbsp;</div>
														<div style="text-align: right; display: inline;">%view_past_questions%</div>
													</div>
												</div>'
					),

					'past_questions_format' => Array(
								'added'     => '0.1',
								'type'      => 'wysiwyg',
								'default'   =>
												'<div style="border: 1px solid #000066; width: 420px; padding: 5px">
													<div style="align: center;">
													   <strong>Past Questions</strong>
													</div>
													<div>
													   <strong>Current Question:</strong><br />
													   %active_question_date%
													</div>
													<div style="list-style: none">
													   %question_list_date_no_active%
													</div>
												</div>'
					)

		);

	}//end constructor


}//end class


?>