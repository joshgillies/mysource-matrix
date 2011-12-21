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
* $Id: page_decision_tree_edit_fns.inc,v 1.1 2011/12/21 02:28:40 mhaidar Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';


/**
* Page_Decision_Tree_Edit_Fns
*
* @author  Mohamed Haidar <mhaidar@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Decision_Tree_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*
	* @return void
	* @access public
	*/
	function __construct()
	{
		parent::__construct();
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor
	
	
	/**
	* Paint the dropdown to add the first question under the page. Once added paints the asset tag line.
	*
	* Return boolean FALSE if attributes write access is not held, TRUE otherwise
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintFirstQuestion(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$questions = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'decision_tree_question', FALSE, 'major', 'first_question');
		if (!empty($questions[0]['minorid'])) {
			echo get_asset_tag_line($questions[0]['minorid'], 'details');
			return TRUE;
		} else {
			if (!$write_access) echo "<i>".translate('first_question_not_created')."</i>";
		}
		
		if (!$write_access) return FALSE;
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$question_types = $am->getTypeDescendants('decision_tree_question');
		$questions = Array('' => '-- Choose Type --');

		foreach ($question_types as $question) {
			$q = str_replace('decision_tree_question_type', '', $question);
			$q = str_replace('_', ' ', $q);
			$q = trim(ucwords($q));
			$questions[$question] = $q;
		}

		combo_box($prefix.'question_type', $questions, FALSE, '');
		echo '&nbsp;';

		text_box($prefix.'question_name', '', '25');
		return TRUE;

	}//end paintAddQuestions()


	/**
	* Process the newly added first question.
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processFirstQuestion(&$asset, &$o, $prefix)
	{
		$type   = (isset($_POST[$prefix.'question_type'])) ? $_POST[$prefix.'question_type'] : '';
		$name = (isset($_POST[$prefix.'question_name'])) ? $_POST[$prefix.'question_name'] : '';

		if ($type && $name) {
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($type);
			$type = ucwords($type);
			$new_asset = new $type;
			$new_asset->setAttrValue('name', $name);
			// Link the asset to the parent asset
			$link = Array(
						'asset'			=> $asset,
						'link_type'		=> SQ_LINK_TYPE_2,
						'value'			=> 'first_question',
						'sort_order'	=> NULL,
						'is_dependant'	=> TRUE,
						'is_exclusive'	=> TRUE,
					);
	
			if(!$new_asset->create($link)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
			
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		}//end if

		return TRUE;

	}//end processFirstQuestion()
	
	
	/**
	* Paints a list of the Result Bodycopies under the Result Bodycopies folder 
	* with their min - max points range.
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintResultBodycopies(&$asset, &$o, $prefix)
	{
	
		$admin_access = $asset->writeAccess('attributes');
		$bodycopy_points = $asset->attr('bodycopy_points');
		$bodycopies = $asset->getResultBodycopies();

		?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-header">
							<?php echo translate('result_bodycopy'); ?>
						</td>
						<td class="sq-backend-table-header">
							<?php echo translate('points_range_min_max_incl'); ?>
						</td>
					</tr>
				<?php
				
				foreach ($bodycopies as $b_link) {
					if ($b_link['value'] == 'default_result_bodycopy') continue;
					$assetid = $b_link['minorid'];
					?>
					<tr>
						<td class="sq-backend-table-cell">
							<?php echo get_asset_tag_line($assetid, 'details'); ?>
						</td>
						<td class="sq-backend-table-cell">
							<?php
								$min = (!empty($bodycopy_points[$assetid]['min'])) ? $bodycopy_points[$assetid]['min'] : 0;
								$max = (!empty($bodycopy_points[$assetid]['max'])) ? $bodycopy_points[$assetid]['max'] : 0;
								if ($admin_access) {
									text_box($prefix.'_bodycopy_points_min_'.$assetid, $min);
									echo "&nbsp;";
									text_box($prefix.'_bodycopy_points_max_'.$assetid, $max);
								} else {
									echo $min.' to '.$max;
								}
							?>
						</td>
					</tr>
					<?php
				}//end foreach data
				?>
				</table>
		<?php
		return TRUE;

	}//end paintResultBodycopies()


	/**
	* Process the mix - max points range for each bodycopy under the Result Bodycopies folder.
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processResultBodycopies(&$asset, &$o, $prefix)
	{
		
		if (!$asset->writeAccess('attributes')) return FALSE;
		
		$bodycopy_points = Array();
		$bodycopies = $asset->getResultBodycopies();
		
		foreach ($bodycopies as $b_link) {
			if ($b_link['value'] == 'default_result_bodycopy') continue;
			$assetid = $b_link['minorid'];
			
			$max = (isset($_POST[$prefix.'_bodycopy_points_max_'.$assetid])) ? $_POST[$prefix.'_bodycopy_points_max_'.$assetid] : 0;
			$min = (isset($_POST[$prefix.'_bodycopy_points_min_'.$assetid])) ? $_POST[$prefix.'_bodycopy_points_min_'.$assetid] : 0;
			
			if (!is_numeric($max) || !is_numeric($min)) {
				trigger_localised_error('CMS0115', E_USER_WARNING);
				return FALSE;
			}
			
			if ($min > $max){
				trigger_localised_error('CMS0116', E_USER_WARNING);	
				return FALSE;
			}
			
			$bodycopy_points[$assetid]['max'] = $max;
			$bodycopy_points[$assetid]['min'] = $min;
		}
		
		$asset->setAttrValue('bodycopy_points', $bodycopy_points);
		$asset->saveAttributes();
		
		return TRUE;

	}//end processResultBodycopies()
	
	
	/**
	* Provides a dropdown to add 0-10 bodycopies under the Result Bodycopies folder.
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAddResultBodycopies(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		
		$num = Array();
		for ($i = 0; $i <= 10; $i++) {
			$num[$i] = $i;
		}
		
		combo_box($prefix.'num_bodycopies', $num, FALSE, '');
		
		return TRUE;
			
	}//end paintAddResultBodycopies()
	
	
	/**
	* Process the addition of bodycopies under the Result Bodycopies folder.
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAddResultBodycopies(&$asset, &$o, $prefix)
	{
		
		$to_create = (isset($_POST[$prefix.'num_bodycopies'])) ? $_POST[$prefix.'num_bodycopies'] : 0;
		$bodycopies = $asset->getResultBodycopies();
		$no_bodycopies = count($bodycopies) - 1; 
		for ($i = 0; $i < $to_create; $i++){
			$no_bodycopies++;
			if (!$asset->createResultBodycopy('Result Bodycopy '.$no_bodycopies, '')){
				trigger_localised_error('CMS0117', E_USER_WARNING);
				return FALSE;	
			}
		}
		
		return TRUE;	
	}//end processAddResultBodycopies()
		


}//end class

?>
