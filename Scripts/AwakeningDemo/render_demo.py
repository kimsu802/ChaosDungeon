"""Run using UnrealEditor -ExecutePythonScript=... (MovieRenderPipeline enabled)."""
import unreal as u
import os

u.EditorPythonScripting.set_keep_python_script_alive(True)
subsystem=u.get_editor_subsystem(u.MoviePipelineQueueSubsystem)
queue=subsystem.get_queue()
queue.delete_all_jobs()
job=queue.allocate_new_job(u.MoviePipelineExecutorJob)
job.job_name='Awakening'
job.map=u.SoftObjectPath('/Game/AwakeningDemo/L_AwakeningDemo.L_AwakeningDemo')
job.sequence=u.SoftObjectPath('/Game/AwakeningDemo/LS_AwakeningDemo.LS_AwakeningDemo')
job.set_configuration(u.load_asset('/Game/AwakeningDemo/MRQ_AwakeningDemo'))
executor=u.MoviePipelinePIEExecutor(subsystem)

def done(executor, success):
    with open(os.path.join(u.Paths.project_dir(),'Saved','AwakeningDemo','render_result.txt'),'w') as f:
        f.write('SUCCESS' if success else 'FAILED')
    u.log('AWAKENING_RENDER_COMPLETE: '+str(success))
    u.SystemLibrary.quit_editor()

executor.on_executor_finished_delegate.add_callable_unique(done)
subsystem.render_queue_with_executor_instance(executor)
