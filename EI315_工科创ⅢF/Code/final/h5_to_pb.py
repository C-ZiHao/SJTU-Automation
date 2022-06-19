from tensorflow import keras
import tensorflow as tf
import numpy as np
import cv2
import os


# 这个函数参考自网上
def freeze_session(session, keep_var_names=None, output_names=None, clear_devices=True):
    """
    Freezes the state of a session into a pruned computation graph.
    Creates a new computation graph where variable nodes are replaced by
    constants taking their current value in the session. The new graph will be
    pruned so subgraphs that are not necessary to compute the requested
    outputs are removed.
    @param session The TensorFlow session to be frozen.
    @param keep_var_names A list of variable names that should not be frozen,
                          or None to freeze all the variables in the graph.
    @param output_names Names of the relevant graph outputs.
    @param clear_devices Remove the device directives from the graph for better portability.
    @return The frozen graph definition.
    """
    graph = session.graph
    with graph.as_default():
        freeze_var_names = list(set(v.op.name for v in tf.global_variables()).difference(keep_var_names or []))
        output_names = output_names or []
        output_names += [v.op.name for v in tf.global_variables()]
        input_graph_def = graph.as_graph_def()
        if clear_devices:
            for node in input_graph_def.node:
                node.device = ''
        frozen_graph = tf.graph_util.convert_variables_to_constants(
            session, input_graph_def, output_names, freeze_var_names)
        return frozen_graph


if __name__ == '__main__':
    # keras训练保存的h5文件
    input_file = 'model.h5'
    output_graph_name = input_file.replace(".h5", ".pb")

    #  加载模型
    keras.backend.set_learning_phase(0)
    h5_model = keras.models.load_model(input_file)
    frozen_graph = freeze_session(keras.backend.get_session(), output_names=[out.op.name for out in h5_model.outputs])
    tf.train.write_graph(frozen_graph, "./", output_graph_name, as_text=False)

    dnn_model = cv2.dnn.readNetFromTensorflow(output_graph_name)
    x = np.zeros([1, 32, 32, 3], dtype=np.float32)
    y_keras = h5_model.predict(x)
    dnn_model.setInput(np.transpose(x, [0, 3, 1, 2]))
    y_dnn = dnn_model.forward()
    print("y_keras: ", y_keras)
    print("y_dnn: ", y_dnn)
