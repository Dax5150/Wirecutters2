class Wirecutter_UserAction : ScriptedUserAction
{
	[Attribute("", UIWidgets.Auto)]
	protected ref SCR_AudioSourceConfiguration m_AudioSourceConfiguration;
	
	[Attribute("0", UIWidgets.ComboBox, "Instrument type", "", ParamEnumArray.FromEnum(SCR_EInstrumentType) )]
	protected SCR_EInstrumentType m_eInstrumentType;
	
	[Attribute("", UIWidgets.Coords)]
	private vector m_vSoundOffset;
	
	
	protected SCR_GadgetManagerComponent m_GadgetManager;
	protected IEntity m_pUser;
	protected AudioHandle m_AudioHandle = AudioHandle.Invalid;
	protected static ref ScriptInvokerInt2 s_onFenceCut;
	
	override void OnActionStart(IEntity pUserEntity) {
		if (s_onFenceCut) {
			s_onFenceCut.Invoke(GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity), m_eInstrumentType);
		}
		
		if (!m_AudioSourceConfiguration || !m_AudioSourceConfiguration.IsValid()) { return; }
		
		const IEntity owner = GetOwner();
		SCR_SoundManagerModule soundManager = SCR_SoundManagerModule.GetInstance(owner.GetWorld());
		if (!soundManager) { return; }
			
		vector worldPosition = owner.CoordToParent(m_vSoundOffset);
		
		SCR_AudioSource audioSource = soundManager.CreateAudioSource(owner, m_AudioSourceConfiguration, worldPosition);
		if (!audioSource) { return; }
					
		AudioSystem.TerminateSound(m_AudioHandle);
		soundManager.PlayAudioSource(audioSource);			
		m_AudioHandle = audioSource.m_AudioHandle;
	}
	
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity) {		
		AudioSystem.TerminateSound(m_AudioHandle);
		
		SCR_SoundManagerModule soundManager = SCR_SoundManagerModule.GetInstance(pOwnerEntity.GetWorld());
		if (!soundManager) { return; }
				
		if (!m_AudioSourceConfiguration || m_AudioSourceConfiguration.m_sSoundProject == string.Empty) { return; }
		
		SCR_AudioSourceConfiguration audioSourceConfiguration = new SCR_AudioSourceConfiguration;
		audioSourceConfiguration.m_sSoundProject = m_AudioSourceConfiguration.m_sSoundProject;
		audioSourceConfiguration.m_eFlags = m_AudioSourceConfiguration.m_eFlags;
		audioSourceConfiguration.m_sSoundEventName = SCR_SoundEvent.SOUND_STOP_PLAYING;
		
		vector worldPosition = pOwnerEntity.CoordToParent(m_vSoundOffset);
				
		SCR_AudioSource audioSource = soundManager.CreateAudioSource(pOwnerEntity, audioSourceConfiguration, worldPosition);
		if (!audioSource) { return; }
					
		soundManager.PlayAudioSource(audioSource);			
		m_AudioHandle = audioSource.m_AudioHandle;
	}
	
	IEntity GetWirecutter(notnull IEntity ent) {
		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(ent);
		if (!gadgetManager) { return null; }
		
		return gadgetManager.GetHeldGadget();
	}
	

	void SetNewGadgetManager(IEntity from, IEntity to) {
		m_GadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(to);
	}
	
	
	override bool CanBeShownScript(IEntity user) {
		if (!m_GadgetManager) {
			m_GadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(user);
			
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
			if (playerController) {
				playerController.m_OnControlledEntityChanged.Insert(SetNewGadgetManager);
			}

			return true;
		};
					
		if (!SCR_WirecutterComponent.Cast(m_GadgetManager.GetHeldGadgetComponent())) { return false; }
		
		return true;
	}
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) {
		if (!Replication.IsServer()) { return; }
		
		SCR_EntityHelper.DeleteEntityAndChildren(pOwnerEntity);
	}

    void ~Wirecutter_UserAction() {
		AudioSystem.TerminateSound(m_AudioHandle);
	}
};