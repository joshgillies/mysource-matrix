<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: decision_tree_question_edit_fns.inc,v 1.3 2012/08/30 00:58:08 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Decision_Tree_Question_Edit_Fns
*
*
* @author  Mohamed Haidar <mhaidar@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Decision_Tree_Question_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Decision_Tree_Question_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
		// override static screens - only display details because the others don't apply
		$this->static_screens = Array(
									'details'	=> Array(
													'name'			=> 'Details',
													'force_unlock'	=> FALSE,
													'lock_type'		=> 'none',
												   ),
								);

	}//end constructor


	/**
	* Paint the links to the questions under this question (i.e potential destination questions).
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintQuestionLinks(&$asset, &$o, $prefix)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$admin_access = $asset->writeAccess('attributes');
		$prefix = $asset->getPrefix();
		$questions = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'decision_tree_question', FALSE);

		?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-header">
							<?php echo translate('question'); ?>
						</td>
						<td class="sq-backend-table-header">
							<?php echo translate('type'); ?>
						</td>
					</tr>
				<?php

				foreach ($questions as $q_link) {
					$assetid = $q_link['minorid'];
					$q = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
					if (is_null($q)) continue;
					?>
					<tr>
						<td class="sq-backend-table-cell">
							<?php echo get_asset_tag_line($assetid, 'details'); ?>
						</td>
						<td class="sq-backend-table-cell">
							<?php
								$q_type = str_replace('Decision_Tree_Question_Type', '', get_class($q));
								$q_type = str_replace('_', ' ', $q_type);
								$q_type = trim(ucwords($q_type));
								echo $q_type;
							?>
						</td>
					</tr>
					<?php
				}//end foreach data
				?>
				</table>
		<?php
		return TRUE;

	}//end paintQuestionLinks()


	/**
	* Process the links to the questions under this question (i.e potential destination questions).
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processQuestionLinks(&$asset, &$o, $prefix)
	{

		return TRUE;

	}//end processQuestionLinks()


	/**
	* Paint the dropdowns to add questions to this section
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
	function paintAddQuestions(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('links')) return FALSE;

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
	* Process the newly added questions
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAddQuestions(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('links')) return FALSE;

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
						'value'			=> '',
						'sort_order'	=> NULL,
						'is_dependant'	=> TRUE,
						'is_exclusive'	=> FALSE,
					);

			if(!$new_asset->create($link)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				//trigger error
				return FALSE;
			}

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		}//end if

		return TRUE;

	}//end processAddQuestions()


	/**
	* Paint lock message telling the user to go to our parent to lock me
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintLockMessage(&$asset, &$o, $prefix)
	{
		$dependant_parent = $GLOBALS['SQ_SYSTEM']->am->getDependantParents($asset->id, '', FALSE, FALSE);
		if (empty($dependant_parent[0])) {
			echo "<p>Locking for this asset is controlled by its parent Decision Tree Page.</p>";
			return TRUE;
		}

		$parent_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($dependant_parent[0]);

		$locks = $GLOBALS['SQ_SYSTEM']->am->getLockInfo($parent_asset->id, 'attributes', TRUE);
		$lock  = $locks['attributes'];

		$full_write_access = $parent_asset->writeAccess('');
		$acquire_lock_access = $parent_asset->canLiveEdit('attributes');

		if (empty($lock)) {
			if ($full_write_access && $acquire_lock_access) {
				echo '<p class="sq-backend-unlocked">The "Attributes" lock to the Decision Tree Page is not currently held.</p>';
			} else {
				echo 'You do not have access to edit "'.$parent_asset->name.'".';
			}

		} else {
			// this asset is currently locked so display message to the user
			$user    = $GLOBALS['SQ_SYSTEM']->am->getAsset($lock['userid']);
			$editing = $GLOBALS['SQ_SYSTEM']->am->getAsset($lock['source_assetid']);

			if (SQ_ROLLBACK_VIEW) {
				$now = strtotime($_SESSION['sq_rollback_view']['rollback_time']);
			} else {
				$now = time();
			}

			if (is_null($lock['expires'])) {
				$expires_in = 'The lock is being held indefinitely and will not expire. The lock must be released manually or be forceably acquired by a System Administrator.';
			} else {
				require_once SQ_FUDGE_PATH.'/general/datetime.inc';
				$expires_in = easy_time_total(($lock['expires'] - $now), TRUE);
				if (!$expires_in) $expires_in = '1 second';
				$expires_in = 'The lock is due to expire in '.$expires_in;
			}

			?>
			<p class="sq-backend-locked-by-<?php echo ($GLOBALS['SQ_SYSTEM']->currentUser($user)) ? 'user' : 'someone-else'; ?>">
			The "Attributes" lock to the Decision Tree Question is held by user "<?php echo htmlspecialchars($user->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET); ?>" at "<?php echo htmlspecialchars($editing->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET); ?>".<br/>
			<?php echo $expires_in; ?>
			</p>
			<?php

		}//end if
		?>

		<p>Locking for this asset is controlled by its parent Decision Tree Page<?php
		?>. Click to access this asset: <?php $href = $parent_asset->getBackendHref('details'); ?><a href="<?php echo $href ?>"><?php echo $parent_asset->name ?></a></p><?php
		return TRUE;

	}//end paintLockMessage()


	/**
	* Process lock message (no implementation)
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processLockMessage(&$asset, &$o, $prefix)
	{
		return TRUE;

	}//end processLockMessage()


}//end class

?>
